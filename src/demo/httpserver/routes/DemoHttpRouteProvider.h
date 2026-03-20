#pragma once
#include "core/EventBus.h"
#include "demo/httpserver/parser/DemoControlCommandParser.h"
#include "modules/http/BaseHttpRouteProvider.h"

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
                R"({"name":"demo-http-provider","routes":["/ping","/status","/control","/practice/start","/practice/stop","/practice/reset","/demo/info"]})");
        });

        registry.add_route("GET", "/hello", [](const HttpRequest&) {
            return make_json_response(
                200,
                R"({"hello":"world"})");
        });

        registry.add_route("POST", "/practice/start", [this](const HttpRequest&) {
            return publish_practice_command("start");
        });

        registry.add_route("POST", "/practice/stop", [this](const HttpRequest&) {
            return publish_practice_command("stop");
        });

        registry.add_route("POST", "/practice/reset", [this](const HttpRequest&) {
            return publish_practice_command("reset");
        });
    }

private:
    HttpResponse publish_practice_command(const std::string& action)
    {
        PracticeCommand cmd;
        cmd.action = action;

        bus_.publish(Event(
            EventType::PRACTICE_COMMAND,
            "DemoHttpRouteProvider",
            cmd
        ));

        return make_json_response(
            202,
            std::string(R"({"practice_action":")") + action + R"(","status":"accepted"})");
    }

    EventBus& bus_;
};
