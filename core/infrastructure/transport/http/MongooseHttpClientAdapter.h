/**
 * MongooseHttpClientAdapter.h
 * 基于 Mongoose 的异步 HTTP client adapter
 */
#pragma once

#include <atomic>
#include <chrono>
#include <deque>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "infrastructure/transport/http/IHttpClientAdapter.h"

struct mg_mgr;
struct mg_connection;
struct mg_http_message;
struct mg_str;

class MongooseHttpClientAdapter : public IHttpClientAdapter
{
public:
    MongooseHttpClientAdapter();
    ~MongooseHttpClientAdapter() override;

    void async_request(
        HttpClientRequest request,
        SuccessHandler on_success,
        FailureHandler on_failure) override;

    void stop() override;

private:
    struct PendingRequest
    {
        HttpClientRequest request;
        SuccessHandler on_success;
        FailureHandler on_failure;
    };

    struct RequestContext
    {
        MongooseHttpClientAdapter* self {nullptr};
        HttpClientRequest request;
        SuccessHandler on_success;
        FailureHandler on_failure;
        std::chrono::steady_clock::time_point deadline;
        bool request_sent {false};
        bool completed {false};
    };

    static void on_mg_event(mg_connection* connection, int ev, void* ev_data);

    void worker_loop();
    void process_pending_requests();
    void start_request(PendingRequest pending);
    void send_http_request(mg_connection* connection, RequestContext& context);
    void handle_http_response(
        mg_connection* connection,
        RequestContext& context,
        const mg_http_message& message);
    void complete_with_failure(RequestContext& context, const std::string& error_message);
    void fail_pending_requests(const std::string& error_message);

    static std::string copy_mg_str(const mg_str& value);
    static std::unordered_map<std::string, std::string> extract_headers(const mg_http_message& message);
    static bool has_header(
        const std::unordered_map<std::string, std::string>& headers,
        const std::string& name);
    static bool equals_ignore_case(const std::string& lhs, const std::string& rhs);

private:
    std::mutex pending_mutex_;
    std::deque<PendingRequest> pending_requests_;

    mg_mgr* mgr_ {nullptr};
    std::thread worker_;
    std::atomic<bool> running_ {false};
};
