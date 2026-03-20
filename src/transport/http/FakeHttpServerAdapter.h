#pragma once
#include <map>
#include <string>
#include <utility>
#include "core/Logger.h"
#include "transport/http/IHttpServerAdapter.h"

class FakeHttpServerAdapter : public IHttpServerAdapter
{
public:
    bool start(const std::string& host, int port) override {
        Logger::info(
            "[FakeHttpServerAdapter] start host=" + host +
            "port=" + std::to_string(port)
        );
        started_ = true;
        return true;
    }

    void stop() override
    {
        Logger::info("[FakeHttpServerAdapter] stop");
        started_ = false;
    }

    void register_handler(
        const std::string& method,
        const std::string& path,
        Handler handler
    ) override {
        handlers_[make_key(method, path)] = std::move(handler);

        Logger::info(
            "[FakeHttpServerAdapter] route registered: " + 
            method + " " + path
        );
    }

    HttpResponse simulate_request(const HttpRequest& req) {
        if (!started_)
        {
            HttpResponse resp;
            resp.status = 503;
            resp.body = R"({"error":"server not started"})";
            resp.headers["Content-Type"] = "application/json";
            return resp;
        } 

        auto it = handlers_.find(make_key(req.method, req.path));
        if (it == handlers_.end())
        {
            HttpResponse resp;
            resp.status = 404;
            resp.body = R"({"error":"not found"})";
            resp.headers["Content-Type"] = "application/json";
            return resp;
        }

        Logger::info(
            "[FakeHttpServerAdapter] simulate request: " +
            req.method + " " + req.path +
            " body=" + req.body
        );

        return it->second(req);
    }

private:
    static std::string make_key(const std::string& method, const std::string& path) {
        return method + " " + path;
    }

private:
    bool started_ {false};
    std::map<std::string, Handler> handlers_;
};
