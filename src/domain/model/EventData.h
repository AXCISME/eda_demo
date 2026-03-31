#pragma once
#include <string>
#include <variant>

struct DeviceSample
{
    std::string device_id;
    float temperature {0.0f};
    float pressure {0.0f};
};

struct ControlCommand
{
    int addr {0};
    int value {0};
};

struct WsClientInfo
{
    std::string client_id;
};

struct WsMessage
{
    std::string client_id;
    std::string text;
};

using EventData = std::variant<
    std::monostate,
    DeviceSample,
    ControlCommand,
    WsClientInfo,
    WsMessage
>;
