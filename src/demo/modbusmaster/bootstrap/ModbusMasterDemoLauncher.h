#pragma once
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
    app.run();
    return 0;
}
