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

struct PracticeCommand
{
    std::string action;
};

struct PracticeState
{
    std::string status {"idle"};
    bool active {false};
    int sample_count {0};
    std::string last_device_id;
    float last_temperature {0.0f};
    float last_pressure {0.0f};
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
    PracticeCommand,
    PracticeState,
    WsClientInfo,
    WsMessage
>;
