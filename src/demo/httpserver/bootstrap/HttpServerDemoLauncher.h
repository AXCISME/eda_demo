#pragma once
#include <chrono>
#include <memory>
#include <thread>
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

    std::thread simulator([&app]() {
        app.simulate_ws_client_connect("client-001");
        std::this_thread::sleep_for(std::chrono::seconds(3));
        app.simulate_ws_client_message("client-001", 101, 0);
        std::this_thread::sleep_for(std::chrono::seconds(57));
        app.stop();
    });

    app.run();

    if (simulator.joinable())
    {
        simulator.join();
    }

    return 0;
}
