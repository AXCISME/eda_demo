#include "transport/http/MongooseHttpServerAdapter.h"

#include <sstream>

#include "core/Logger.h"
#include "transport/http/HttpRequest.h"
#include "transport/http/HttpResponse.h"

#include "mongoose.h"

MongooseHttpServerAdapter::MongooseHttpServerAdapter() = default;

MongooseHttpServerAdapter::~MongooseHttpServerAdapter()
{
    stop();
}

bool MongooseHttpServerAdapter::start(const std::string& host, int port)
{
    if (running_)
    {
        Logger::warn("[MongooseHttpServerAdapter] already running");
        return true;
    }

    listen_url_ = "http://" + host + ":" + std::to_string(port);

    mgr_ = new mg_mgr();
    mg_mgr_init(mgr_);

    listener_ = mg_http_listen(mgr_, listen_url_.c_str(), &MongooseHttpServerAdapter::on_mg_event, this);
    if (listener_ == nullptr)
    {
        Logger::error("[MongooseHttpServerAdapter] mg_http_listen failed: " + listen_url_);
        mg_mgr_free(mgr_);
        delete mgr_;
        mgr_ = nullptr;
        return false;
    }

    running_ = true;

    worker_ = std::thread([this]() {
        Logger::info("[MongooseHttpServerAdapter] started at " + listen_url_);

        while (running_)
        {
            mg_mgr_poll(mgr_, 100);
        }

        Logger::info("[MongooseHttpServerAdapter] poll loop stopped");
    });

    return true;
}

void MongooseHttpServerAdapter::stop()
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

    if (mgr_ != nullptr)
    {
        mg_mgr_free(mgr_);
        delete mgr_;
        mgr_ = nullptr;
        listener_ = nullptr;
    }

    Logger::info("[MongooseHttpServerAdapter] stopped");
}

void MongooseHttpServerAdapter::register_handler(
    const std::string& method,
    const std::string& path,
    Handler handler)
{
    handlers_[make_key(method, path)] = std::move(handler);

    Logger::info(
        "[MongooseHttpServerAdapter] route registered: " +
        method + " " + path
    );
}

void MongooseHttpServerAdapter::on_mg_event(mg_connection* c, int ev, void* ev_data)
{
    if (ev == MG_EV_HTTP_MSG)
    {
        auto* self = static_cast<MongooseHttpServerAdapter*>(c->fn_data);
        auto* hm = static_cast<mg_http_message*>(ev_data);
        if (self != nullptr)
        {
            self->handle_http_message(c, hm);
        }
    }
}

void MongooseHttpServerAdapter::handle_http_message(mg_connection* c, mg_http_message* hm)
{
    HttpRequest req;
    req.method = std::string(hm->method.buf, hm->method.len);
    req.path = std::string(hm->uri.buf, hm->uri.len);
    req.query_string = std::string(hm->query.buf, hm->query.len);
    req.body = std::string(hm->body.buf, hm->body.len);

    for (int i = 0; i < MG_MAX_HTTP_HEADERS; ++i)
    {
        if (hm->headers[i].name.len == 0)
        {
            break;
        }

        std::string name(hm->headers[i].name.buf, hm->headers[i].name.len);
        std::string value(hm->headers[i].value.buf, hm->headers[i].value.len);
        req.headers[name] = value;
    }

    Logger::info(
        "[MongooseHttpServerAdapter] request: " +
        req.method + " " + req.path +
        (req.query_string.empty() ? "" : "?" + req.query_string) +
        " body=" + req.body
    );

    HttpResponse resp;
    auto it = handlers_.find(make_key(req.method, req.path));
    if (it == handlers_.end())
    {
        resp.status = 404;
        resp.body = R"({"error":"not found"})";
        resp.headers["Content-Type"] = "application/json";
    }
    else
    {
        resp = it->second(req);
    }

    std::ostringstream headers_stream;
    for (const auto& kv : resp.headers)
    {
        headers_stream << kv.first << ": " << kv.second << "\r\n";
    }

    std::string headers = headers_stream.str();

    mg_http_reply(
        c,
        resp.status,
        headers.c_str(),
        "%.*s",
        static_cast<int>(resp.body.size()),
        resp.body.data()
    );
}

std::string MongooseHttpServerAdapter::make_key(const std::string& method, const std::string& path) const
{
    return method + " " + path;
}
