// src/modules/http/BaseHttpRouteProvider.h
#pragma once
#include "modules/http/IHttpRouteProvider.h"
#include "modules/http/HttpResponses.h"

class BaseHttpRouteProvider : public IHttpRouteProvider
{
public:
    void register_routes(IHttpRouteRegistry& registry) override
    {
        registry.add_route("GET", "/ping", [this](const HttpRequest& req) {
            return on_ping(req);
        });

        registry.add_route("GET", "/status", [this](const HttpRequest& req) {
            return on_status(req);
        });

        registry.add_route("POST", "/control", [this](const HttpRequest& req) {
            return on_control(req);
        });

        register_custom_routes(registry);
    }

protected:
    virtual HttpResponse on_ping(const HttpRequest&)
    {
        return make_json_response(200, R"({"message":"pong"})");
    }

    virtual HttpResponse on_status(const HttpRequest&)
    {
        return make_json_response(200, R"({"status":"ok"})");
    }

    virtual HttpResponse on_control(const HttpRequest&)
    {
        return make_json_response(501, R"({"error":"control not implemented"})");
    }

    virtual void register_custom_routes(IHttpRouteRegistry&)
    {
    }
};
