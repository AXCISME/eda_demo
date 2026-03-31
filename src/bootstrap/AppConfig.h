#pragma once
#include <string>

struct AppConfig
{
    bool enable_http {true};
    std::string http_host {"0.0.0.0"};
    int http_port {8080};
    std::string http_backend {"mongoose"};
    int modbus_poll_interval_ms {1000};
};

