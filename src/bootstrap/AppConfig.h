/**
 * AppConfig.h
 * 定义运行期配置模型。
 * 用户想怎么启动协议，选哪个backend、端口号是多少
 */
#pragma once

#include <string>

enum class HttpBackendId
{
    NONE = 0,
    FAKE,
    MONGOOSE
};

enum class ModbusBackendId
{
    NONE = 0,
    FAKE
};

inline const char* to_string(HttpBackendId backend)
{
    switch (backend)
    {
        case HttpBackendId::FAKE: return "fake";
        case HttpBackendId::MONGOOSE: return "mongoose";
        default: return "none";
    }
}

inline const char* to_string(ModbusBackendId backend)
{
    switch (backend)
    {
        case ModbusBackendId::FAKE: return "fake";
        default: return "none";
    }
}

struct HttpConfig
{
    bool enabled {true};
    HttpBackendId backend {HttpBackendId::MONGOOSE};
    std::string host {"0.0.0.0"};
    int port {8080};
};

struct ModbusConfig
{
    bool enabled {true};
    ModbusBackendId backend {ModbusBackendId::FAKE};
    int poll_interval_ms {1000};
    int slave_id {1};
    int sample_base_addr {100};
};

struct AppConfig
{
    HttpConfig http;
    ModbusConfig modbus;
};
