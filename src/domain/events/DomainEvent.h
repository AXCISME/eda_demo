#pragma once

#include <string>
#include <chrono>
#include <cstdint>
#include <utility>

#include "domain/model/EventData.h"

enum class EventCategory {
    NONE = 0,
    RUNTIME,
    PROTOCOL,
    DOMAIN
};

enum class EventType {
    NONE = 0,                      // 空事件

    // Runtime events
    SHUTDOWN,                      // 系统关闭

    // Protocol / integration events
    MODBUS_SAMPLE_RECEIVED,        
    WS_CLIENT_CONNECTED,           // WebSocket客户端连接
    WS_MESSAGE_RECEIVED,           // WebSocket消息接收
    WS_BROADCAST,                  // WebSocket广播

    // Domain / application events
    TELEMETRY_UPDATED,             // 数据更新
    CONTROL_COMMAND                // 控制命令
};

inline EventCategory category_of(EventType type)
{
    switch (type)
    {
        case EventType::SHUTDOWN:
            return EventCategory::RUNTIME;

        case EventType::MODBUS_SAMPLE_RECEIVED:
        case EventType::WS_CLIENT_CONNECTED:
        case EventType::WS_MESSAGE_RECEIVED:
        case EventType::WS_BROADCAST:
            return EventCategory::PROTOCOL;

        case EventType::TELEMETRY_UPDATED:
        case EventType::CONTROL_COMMAND:
            return EventCategory::DOMAIN;

        default:
            return EventCategory::NONE;
    }
}

inline const char* to_string(EventCategory category)
{
    switch (category)
    {
        case EventCategory::RUNTIME: return "RUNTIME";
        case EventCategory::PROTOCOL: return "PROTOCOL";
        case EventCategory::DOMAIN: return "DOMAIN";
        default: return "NONE";
    }
}

inline const char* to_string(EventType type)
{
    switch (type)
    {
        case EventType::SHUTDOWN: return "SHUTDOWN";
        case EventType::MODBUS_SAMPLE_RECEIVED: return "MODBUS_SAMPLE_RECEIVED";
        case EventType::WS_CLIENT_CONNECTED: return "WS_CLIENT_CONNECTED";
        case EventType::WS_MESSAGE_RECEIVED: return "WS_MESSAGE_RECEIVED";
        case EventType::WS_BROADCAST: return "WS_BROADCAST";
        case EventType::TELEMETRY_UPDATED: return "TELEMETRY_UPDATED";
        case EventType::CONTROL_COMMAND: return "CONTROL_COMMAND";
        default: return "NONE";
    }
}

struct Event {
    EventType type {EventType::NONE};
    std::string source;
    EventData data;
    uint64_t timestamp_ms {0};

    Event() = default;

    Event(EventType t, std::string s, EventData d = {})
        : type(t), source(std::move(s)), data(std::move(d))
    {
        timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>( // 转换成毫秒单位
            std::chrono::system_clock::now().time_since_epoch()  // 当前系统时间距离时钟起点的时间长度
        ).count();  // 取出毫秒数本身，变成一个整数值
    }
};
