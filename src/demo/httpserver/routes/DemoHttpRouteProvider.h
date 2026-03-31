#pragma once
#include "runtime/bus/EventBus.h"
#include "demo/httpserver/parser/DemoControlCommandParser.h"
#include "interfaces/http/BaseHttpRouteProvider.h"

class DemoHttpRouteProvider : public BaseHttpRouteProvider
{
public:
    explicit DemoHttpRouteProvider(EventBus& bus)
        : bus_(bus)
    {
    }

protected:
    void register_custom_routes(IHttpRouteRegistry& registry) override
    {
        registry.add_route("GET", "/demo/info", [](const HttpRequest&) {
            return make_json_response(
                200,
                R"({"name":"demo-http-provider","routes":["/ping","/status","/control","/demo/info"]})");
        });

        registry.add_route("GET", "/hello", [](const HttpRequest&) {
            return make_json_response(
                200,
                R"({"hello":"world"})");
        });
    }

private:
    EventBus& bus_;
};
