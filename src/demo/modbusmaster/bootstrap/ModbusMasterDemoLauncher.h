#pragma once

#include <chrono>
#include <thread>
#include <vector>

#include "application/services/ControlService.h"
#include "bootstrap/ApplicationBootstrap.h"
#include "runtime/logging/Logger.h"

inline int run_modbusmaster_demo()
{
    AppConfig config;
    config.http.enabled = false;
    config.http.backend = HttpBackendId::FAKE;
    config.http.host = "0.0.0.0";
    config.http.port = 8080;
    config.modbus_master.enabled = true;
    config.modbus_master.backend = ModbusMasterBackendId::FAKE;

    auto host = ApplicationBootstrap::create(
        config,
        {},
        {},
        [](EventBus& bus) -> std::vector<std::unique_ptr<IBusinessModule>> {
            std::vector<std::unique_ptr<IBusinessModule>> modules;
            modules.push_back(std::make_unique<ControlService>(bus));
            return modules;
        });
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
