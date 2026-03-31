#pragma once

#include <chrono>
#include <thread>

#include "bootstrap/ApplicationBootstrap.h"
#include "runtime/logging/Logger.h"

inline int run_modbusmaster_demo()
{
    AppConfig config;
    config.enable_http = false;
    config.http_backend = "fake";
    config.http_host = "0.0.0.0";
    config.http_port = 8080;

    auto host = ApplicationBootstrap::create(config);
    host->init();

    Logger::info("[ModbusMasterDemo] running for 60 seconds");

    std::thread stopper([app = host.get()]() {
        std::this_thread::sleep_for(std::chrono::seconds(60));
        app->stop();
    });

    host->run();

    if (stopper.joinable())
    {
        stopper.join();
    }

    return 0;
}
