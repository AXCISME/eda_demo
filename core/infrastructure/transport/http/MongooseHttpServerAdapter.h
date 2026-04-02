/**
 * MongooseHttpServerAdapter.h
 * 开源库Mongoose的适配器，启动、停止、路由注册、消息解析
 */
#pragma once
#include <atomic>
#include <map>
#include <string>
#include <thread>

#include "infrastructure/transport/http/IHttpServerAdapter.h"

struct mg_mgr;
struct mg_connection;
struct mg_http_message;

class MongooseHttpServerAdapter : public IHttpServerAdapter {
public:
    MongooseHttpServerAdapter();
    ~MongooseHttpServerAdapter() override;

    bool start(const std::string& host, int port) override;
    void stop() override;

    void register_handler(
        const std::string& method,
        const std::string& path,
        Handler handler
    ) override;

private:
    static void on_mg_event(mg_connection* c, int ev, void* ev_data);

    void handle_http_message(mg_connection* c, mg_http_message* hm);
    std::string make_key(const std::string& method, const std::string& path) const;

private:
    std::map<std::string, Handler> handlers_;
    std::string listen_url_;

    mg_mgr* mgr_ {nullptr};
    mg_connection* listener_ {nullptr};

    std::thread worker_;
    std::atomic<bool> running_ {false};
};
