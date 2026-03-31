#pragma once

#include <chrono>
#include <thread>

#include "bootstrap/ApplicationBootstrap.h"
#include "runtime/logging/Logger.h"

inline int run_modbusmaster_demo()
{
    AppConfig config;
    config.http.enabled = false;
    config.http.backend = HttpBackendId::FAKE;
    config.http.host = "0.0.0.0";
    config.http.port = 8080;
    config.modbus.enabled = true;
    config.modbus.backend = ModbusBackendId::FAKE;

    auto host = ApplicationBootstrap::create(config);
    if (!host)
    {
        Logger::error("[ModbusMasterDemo] application bootstrap failed");
        return 1;
    }
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
