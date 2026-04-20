#include "infrastructure/transport/http/MongooseHttpClientAdapter.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <utility>
#include <vector>

#include "runtime/logging/Logger.h"

#include "mongoose.h"

MongooseHttpClientAdapter::MongooseHttpClientAdapter()
{
    mgr_ = new mg_mgr();
    mg_mgr_init(mgr_);

    running_ = true;
    worker_ = std::thread([this]() {
        worker_loop();
    });
}

MongooseHttpClientAdapter::~MongooseHttpClientAdapter()
{
    stop();
}

void MongooseHttpClientAdapter::async_request(
    HttpClientRequest request,
    SuccessHandler on_success,
    FailureHandler on_failure)
{
    if (!running_ || mgr_ == nullptr)
    {
        if (on_failure)
        {
            HttpClientFailure failure;
            failure.request_id = request.request_id;
            failure.error_message = "mongoose http client adapter is stopped";
            on_failure(std::move(failure));
        }
        return;
    }

    std::lock_guard<std::mutex> lock(pending_mutex_);
    pending_requests_.push_back(PendingRequest {
        std::move(request),
        std::move(on_success),
        std::move(on_failure)
    });
}

void MongooseHttpClientAdapter::stop()
{
    if (!running_)
    {
        return;
    }

    running_ = false;

    if (worker_.joinable())
    {
        worker_.join();
    }
}

void MongooseHttpClientAdapter::on_mg_event(mg_connection* connection, int ev, void* ev_data)
{
    auto* context = static_cast<RequestContext*>(connection->fn_data);
    if (context == nullptr || context->self == nullptr)
    {
        return;
    }

    switch (ev)
    {
        case MG_EV_CONNECT:
            if (!connection->is_tls)
            {
                context->self->send_http_request(connection, *context);
            }
            break;

        case MG_EV_TLS_HS:
            context->self->send_http_request(connection, *context);
            break;

        case MG_EV_HTTP_MSG:
            context->self->handle_http_response(
                connection,
                *context,
                *static_cast<mg_http_message*>(ev_data));
            break;

        case MG_EV_ERROR:
            context->self->complete_with_failure(
                *context,
                ev_data != nullptr
                    ? static_cast<const char*>(ev_data)
                    : "mongoose connection error");
            connection->is_closing = 1;
            break;

        case MG_EV_POLL:
            if (!context->completed && std::chrono::steady_clock::now() >= context->deadline)
            {
                context->self->complete_with_failure(*context, "http request timed out");
                connection->is_closing = 1;
            }
            break;

        case MG_EV_CLOSE:
            if (!context->completed)
            {
                context->self->complete_with_failure(*context, "connection closed before response");
            }
            delete context;
            connection->fn_data = nullptr;
            break;

        default:
            break;
    }
}

void MongooseHttpClientAdapter::worker_loop()
{
    Logger::info("[MongooseHttpClientAdapter] worker started");

    while (running_)
    {
        process_pending_requests();
        mg_mgr_poll(mgr_, 50);
    }

    fail_pending_requests("mongoose http client adapter stopped");

    if (mgr_ != nullptr)
    {
        mg_mgr_free(mgr_);
        delete mgr_;
        mgr_ = nullptr;
    }

    Logger::info("[MongooseHttpClientAdapter] worker stopped");
}

void MongooseHttpClientAdapter::process_pending_requests()
{
    std::vector<PendingRequest> ready_requests;

    {
        std::lock_guard<std::mutex> lock(pending_mutex_);
        while (!pending_requests_.empty())
        {
            ready_requests.push_back(std::move(pending_requests_.front()));
            pending_requests_.pop_front();
        }
    }

    for (auto& pending : ready_requests)
    {
        start_request(std::move(pending));
    }
}

void MongooseHttpClientAdapter::start_request(PendingRequest pending)
{
    if (pending.request.url.empty())
    {
        if (pending.on_failure)
        {
            HttpClientFailure failure;
            failure.request_id = pending.request.request_id;
            failure.error_message = "request url is empty";
            pending.on_failure(std::move(failure));
        }
        return;
    }

    auto* context = new RequestContext();
    context->self = this;
    context->request = std::move(pending.request);
    context->on_success = std::move(pending.on_success);
    context->on_failure = std::move(pending.on_failure);
    context->deadline =
        std::chrono::steady_clock::now()
        + std::chrono::milliseconds(std::max(1, context->request.timeout_ms));

    mg_connection* connection = mg_http_connect(
        mgr_,
        context->request.url.c_str(),
        &MongooseHttpClientAdapter::on_mg_event,
        context);

    if (connection == nullptr)
    {
        complete_with_failure(*context, "mg_http_connect returned null");
        delete context;
        return;
    }

    Logger::info(
        "[MongooseHttpClientAdapter] queued request: "
        + context->request.method + " "
        + context->request.url
        + " request_id=" + context->request.request_id);
}

void MongooseHttpClientAdapter::send_http_request(mg_connection* connection, RequestContext& context)
{
    if (context.request_sent || context.completed)
    {
        return;
    }

    std::string method = context.request.method.empty() ? "GET" : context.request.method;
    std::string uri = mg_url_uri(context.request.url.c_str());
    const mg_str host = mg_url_host(context.request.url.c_str());

    std::ostringstream request_stream;
    request_stream << method << " " << uri << " HTTP/1.1\r\n";

    if (!has_header(context.request.headers, "Host"))
    {
        request_stream << "Host: " << copy_mg_str(host) << "\r\n";
    }

    if (!has_header(context.request.headers, "Connection"))
    {
        request_stream << "Connection: close\r\n";
    }

    if (!has_header(context.request.headers, "Content-Length"))
    {
        request_stream << "Content-Length: " << context.request.body.size() << "\r\n";
    }

    for (const auto& header : context.request.headers)
    {
        request_stream << header.first << ": " << header.second << "\r\n";
    }

    request_stream << "\r\n";
    request_stream << context.request.body;

    const std::string raw_request = request_stream.str();
    mg_send(connection, raw_request.data(), raw_request.size());
    context.request_sent = true;

    Logger::info(
        "[MongooseHttpClientAdapter] request sent: "
        + method + " "
        + context.request.url
        + " request_id=" + context.request.request_id);
}

void MongooseHttpClientAdapter::handle_http_response(
    mg_connection* connection,
    RequestContext& context,
    const mg_http_message& message)
{
    if (context.completed)
    {
        connection->is_closing = 1;
        return;
    }

    context.completed = true;

    if (context.on_success)
    {
        HttpClientResponse response;
        response.request_id = context.request.request_id;
        response.status = mg_http_status(&message);
        response.body = copy_mg_str(message.body);
        response.headers = extract_headers(message);
        context.on_success(std::move(response));
    }

    context.on_success = nullptr;
    context.on_failure = nullptr;
    connection->is_closing = 1;
}

void MongooseHttpClientAdapter::complete_with_failure(
    RequestContext& context,
    const std::string& error_message)
{
    if (context.completed)
    {
        return;
    }

    context.completed = true;

    Logger::error(
        "[MongooseHttpClientAdapter] request failed: request_id="
        + context.request.request_id
        + " error=" + error_message);

    if (context.on_failure)
    {
        HttpClientFailure failure;
        failure.request_id = context.request.request_id;
        failure.error_message = error_message;
        context.on_failure(std::move(failure));
    }

    context.on_success = nullptr;
    context.on_failure = nullptr;
}

void MongooseHttpClientAdapter::fail_pending_requests(const std::string& error_message)
{
    std::deque<PendingRequest> pending;

    {
        std::lock_guard<std::mutex> lock(pending_mutex_);
        pending.swap(pending_requests_);
    }

    while (!pending.empty())
    {
        PendingRequest request = std::move(pending.front());
        pending.pop_front();

        if (request.on_failure)
        {
            HttpClientFailure failure;
            failure.request_id = request.request.request_id;
            failure.error_message = error_message;
            request.on_failure(std::move(failure));
        }
    }
}

std::string MongooseHttpClientAdapter::copy_mg_str(const mg_str& value)
{
    return value.buf == nullptr ? std::string() : std::string(value.buf, value.len);
}

std::unordered_map<std::string, std::string> MongooseHttpClientAdapter::extract_headers(
    const mg_http_message& message)
{
    std::unordered_map<std::string, std::string> headers;

    for (int i = 0; i < MG_MAX_HTTP_HEADERS; ++i)
    {
        if (message.headers[i].name.len == 0)
        {
            break;
        }

        headers.emplace(
            copy_mg_str(message.headers[i].name),
            copy_mg_str(message.headers[i].value));
    }

    return headers;
}

bool MongooseHttpClientAdapter::has_header(
    const std::unordered_map<std::string, std::string>& headers,
    const std::string& name)
{
    return std::any_of(
        headers.begin(),
        headers.end(),
        [&name](const auto& entry) {
            return equals_ignore_case(entry.first, name);
        });
}

bool MongooseHttpClientAdapter::equals_ignore_case(const std::string& lhs, const std::string& rhs)
{
    return lhs.size() == rhs.size()
        && std::equal(
            lhs.begin(),
            lhs.end(),
            rhs.begin(),
            [](unsigned char left, unsigned char right) {
                return std::tolower(left) == std::tolower(right);
            });
}
