#pragma once
#include <string>
#include <chrono>
#include "model/EventData.h"

enum class EventType {
    NONE = 0,                      // 空事件

    // Modbus协议相关事件
    MODBUS_POLL,                   // Modbus轮询
    MODBUS_DATA_RECEIVED,          // Modbus数据接收
    MODBUS_WRITE_REQUEST,          // Modbus写入请求

    // WebSocket相关事件
    WS_CLIENT_CONNECTED,           // WebSocket客户端连接
    WS_MESSAGE_RECEIVED,           // WebSocket消息接收
    WS_BROADCAST,                  // WebSocket广播

    // 数据和控制相关事件
    DATA_UPDATED,                  // 数据更新
    CONTROL_COMMAND,               // 控制命令
    PRACTICE_COMMAND,              // 实践管理命令
    PRACTICE_STATE_CHANGED,        // 实践状态变化

    // 系统事件
    SHUTDOWN                       // 系统关闭
};

inline const char* to_string(EventType type) {
    switch (type)
    {
        case EventType::MODBUS_POLL: return "MODBUS_POLL";
        case EventType::MODBUS_DATA_RECEIVED: return "MODBUS_DATA_RECEIVED";
        case EventType::MODBUS_WRITE_REQUEST: return "MODBUS_WRITE_REQUEST";
        case EventType::WS_CLIENT_CONNECTED: return "WS_CLIENT_CONNECTED";
        case EventType::WS_MESSAGE_RECEIVED: return "WS_MESSAGE_RECEIVED";
        case EventType::WS_BROADCAST: return "WS_BROADCAST";
        case EventType::DATA_UPDATED: return "DATA_UPDATED";
        case EventType::CONTROL_COMMAND: return "CONTROL_COMMAND";
        case EventType::PRACTICE_COMMAND: return "PRACTICE_COMMAND";
        case EventType::PRACTICE_STATE_CHANGED: return "PRACTICE_STATE_CHANGED";
        case EventType::SHUTDOWN: return "SHUTDOWN";
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
