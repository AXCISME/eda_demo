#pragma once
#include <chrono>
#include <thread>
#include "app/Application.h"

inline int run_modbusmaster_demo()
{
    AppConfig config;
    config.enable_http = false;
    config.http_backend = "fake";
    config.http_host = "0.0.0.0";
    config.http_port = 8080;

    Application app(config);
    app.init();

    std::thread simulator([&app]() {
        app.simulate_ws_client_connect("client-001");
        app.simulate_ws_text_message("client-001", "practice:start");
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
