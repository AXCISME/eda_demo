#pragma once

#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include "application/services/ControlService.h"
#include "bootstrap/ApplicationBootstrap.h"
#include "demo/httpserver/routes/DemoHttpRouteProvider.h"
#include "runtime/logging/Logger.h"

inline int run_httpserver_demo()
{
    AppConfig config;
    config.http.enabled = true;
    config.http.backend = HttpBackendId::MONGOOSE;
    config.http.host = "0.0.0.0";
    config.http.port = 8080;

    auto host = ApplicationBootstrap::create(
        config,
        [](EventBus& bus) -> std::unique_ptr<IHttpRouteProvider> {
            return std::make_unique<DemoHttpRouteProvider>(bus);
        },
        {},
        [](EventBus& bus) -> std::vector<std::unique_ptr<IBusinessModule>> {
            std::vector<std::unique_ptr<IBusinessModule>> modules;
            modules.push_back(std::make_unique<ControlService>(bus));
            return modules;
        });
    if (!host)
    {
        Logger::error("[HttpServerDemo] application bootstrap failed");
        return 1;
    }

    host->init();
    host->simulate_ws_client_connect("client-001");

    std::thread simulator([app = host.get()]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        app->simulate_ws_client_message("client-001", 101, 0);

        std::this_thread::sleep_for(std::chrono::seconds(57));
        app->stop();
    });

    Logger::info("[HttpServerDemo] HTTP server ready on :" + std::to_string(config.http.port));
    Logger::info("[HttpServerDemo] try:");
    Logger::info("[HttpServerDemo]   GET  /ping");
    Logger::info("[HttpServerDemo]   GET  /status");
    Logger::info("[HttpServerDemo]   POST /control  body: addr=100&value=1");
    Logger::info("[HttpServerDemo]   GET  /demo/info");
    Logger::info("[HttpServerDemo]   GET  /hello");

    host->run();

    if (simulator.joinable())
    {
        simulator.join();
    }

    host->destroy();

    return 0;
}
