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

enum class HttpClientBackendId
{
    NONE = 0,
    FAKE,
    MONGOOSE
};

enum class ModbusMasterBackendId
{
    NONE = 0,
    FAKE,
    LIBMODBUS_MASTER_TCP,
    LIBMODBUS_MASTER_RTU
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

inline const char* to_string(HttpClientBackendId backend)
{
    switch (backend)
    {
        case HttpClientBackendId::FAKE: return "fake";
        case HttpClientBackendId::MONGOOSE: return "mongoose";
        default: return "none";
    }
}

inline const char* to_string(ModbusMasterBackendId backend)
{
    switch (backend)
    {
        case ModbusMasterBackendId::FAKE: return "fake";
        case ModbusMasterBackendId::LIBMODBUS_MASTER_TCP: return "libmodbus_master_tcp";
        case ModbusMasterBackendId::LIBMODBUS_MASTER_RTU: return "libmodbus_master_rtu";
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

struct HttpClientConfig
{
    bool enabled {false};
    HttpClientBackendId backend {HttpClientBackendId::FAKE};
};

struct ModbusMasterConfig
{
    bool enabled {true};
    ModbusMasterBackendId backend {ModbusMasterBackendId::FAKE};
    int poll_interval_ms {1000};
    int slave_id {1};
    int sample_base_addr {100};
    std::string tcp_host {"127.0.0.1"};
    int tcp_port {502};
    std::string serial_device {"/dev/ttyS1"};
    int baudrate {9600};
    char parity {'N'};
    int data_bits {8};
    int stop_bits {1};
};

struct AppConfig
{
    HttpConfig http;
    HttpClientConfig http_client;
    ModbusMasterConfig modbus_master;
};
