#pragma once
#include <memory>
#include "app/Application.h"
#include "demo/httpserver/routes/DemoHttpRouteProvider.h"

inline int run_httpserver_demo()
{
    AppConfig config;
    config.enable_http = true;
    config.http_backend = "mongoose";
    config.http_host = "0.0.0.0";
    config.http_port = 8080;

    Application app(
        config,
        [](EventBus& bus) -> std::unique_ptr<IHttpRouteProvider> {
            return std::make_unique<DemoHttpRouteProvider>(bus);
        });

    app.init();
    app.run();
    return 0;
}
