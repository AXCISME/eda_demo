#pragma once

#include <any>
#include <string>
#include <typeinfo>

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

using EventData = std::any;

template<typename Payload>
struct EventPayloadFormatter
{
    static std::string to_string(const Payload&)
    {
        return std::string("{payload_type=") + typeid(Payload).name() + "}";
    }
};

template<>
struct EventPayloadFormatter<DeviceSample>
{
    static std::string to_string(const DeviceSample& d)
    {
        return "DeviceSample{device_id=" + d.device_id
            + ", temperature=" + std::to_string(d.temperature)
            + ", pressure=" + std::to_string(d.pressure)
            + "}";
    }
};

template<>
struct EventPayloadFormatter<ControlCommand>
{
    static std::string to_string(const ControlCommand& c)
    {
        return "ControlCommand{addr=" + std::to_string(c.addr)
            + ", value=" + std::to_string(c.value)
            + "}";
    }
};

template<>
struct EventPayloadFormatter<WsClientInfo>
{
    static std::string to_string(const WsClientInfo& c)
    {
        return "WsClientInfo{client_id=" + c.client_id + "}";
    }
};

template<>
struct EventPayloadFormatter<WsMessage>
{
    static std::string to_string(const WsMessage& m)
    {
        return "WsMessage{client_id=" + m.client_id + ", text=" + m.text + "}";
    }
};

template<typename Payload>
inline std::string format_event_payload(const Payload& payload)
{
    return EventPayloadFormatter<Payload>::to_string(payload);
}

inline std::string format_event_payload(const EventData& data)
{
    if (!data.has_value())
    {
        return "{}";
    }

    if (const auto* value = std::any_cast<DeviceSample>(&data))
    {
        return format_event_payload(*value);
    }

    if (const auto* value = std::any_cast<ControlCommand>(&data))
    {
        return format_event_payload(*value);
    }

    if (const auto* value = std::any_cast<WsClientInfo>(&data))
    {
        return format_event_payload(*value);
    }

    if (const auto* value = std::any_cast<WsMessage>(&data))
    {
        return format_event_payload(*value);
    }

    return std::string("{payload_type=") + data.type().name() + "}";
}
