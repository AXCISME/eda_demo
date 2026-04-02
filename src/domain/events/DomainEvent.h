#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

#include "domain/model/EventData.h"

/**
 * 事件一级分类
 */
enum class EventCategory {
    NONE = 0,           // 表示空事件或未分类事件
    RUNTIME,            // 框架运行时事件
    COMMUNICATION,      // 通信/集成层事件，即框架基础能力事件
    BUSINESS            // 业务事件，由具体业务模块自己定义
};

/**
 * 事件标识，系统唯一表示一种事件的键
 * category: 事件大类
 * name: 事件具体名字
 * operator==: 用于比较两个事件类型是不是同一事件类型
 */
struct EventType
{
    EventCategory category {EventCategory::NONE};
    std::string name {"NONE"};

    bool operator==(const EventType& other) const
    {
        return category == other.category && name == other.name;
    }
};

/**
 * 构造事件
 */
inline EventType make_event_type(EventCategory category, std::string name)
{
    return EventType {category, std::move(name)};
}

/**
 * 返回事件的一级分类
 */
inline EventCategory category_of(const EventType& type)
{
    return type.category;
}

/**
 * 返回事件一级分类的名称
 */
inline const char* to_string(EventCategory category)
{
    switch (category)
    {
        case EventCategory::RUNTIME: return "RUNTIME";
        case EventCategory::COMMUNICATION: return "COMMUNICATION";
        case EventCategory::BUSINESS: return "BUSINESS";
        default: return "NONE";
    }
}

/**
 * 返回事件名字
 */
inline const std::string& to_string(const EventType& type)
{
    return type.name;
}

/**
 * 定义一个“泛型结构体”，可以装任意类型的 Payload
 * 实例化时需要指定Payload类型 例如：TypedEvent<int>
 */
template<typename Payload>
struct TypedEvent
{
    EventType type;
};

/**
 * 运行时真正流转的一条事件对象
 * type: 事件标识{一级类型 + 名称}
 * source: 事件发布来源
 * data: 事件数据，实际装的是payload
 * payload_text: 辅助信息
 * timestamp_ms: 记录事件发布时间
 */
struct Event {
    EventType type {make_event_type(EventCategory::NONE, "NONE")};
    std::string source;
    EventData data;
    std::string payload_text {"{}"};
    uint64_t timestamp_ms {0};

    Event() = default;

    // 用于创建没有payload的事件
    Event(EventType t, std::string s)
        : type(std::move(t)),
          source(std::move(s))
    {
        stamp_now();
    }

    // 用于创建带 payload 的事件
    template<typename Payload>
    Event(EventType t, std::string s, Payload payload)
        : type(std::move(t)),
          source(std::move(s)),
          data(std::move(payload))
    {
        using DecayedPayload = std::decay_t<Payload>;
        if (const auto* value = std::any_cast<DecayedPayload>(&data))
        {
            payload_text = format_event_payload(*value);
        }
        stamp_now();
    }

private:
    void stamp_now()
    {
        timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }
};

/**
 * 创建带数据的事件
 */
template<typename Payload>
inline Event make_event(const TypedEvent<Payload>& definition, std::string source, Payload payload)
{
    return Event(definition.type, std::move(source), std::move(payload));
}

/**
 * 创建无数据的事件
 */
inline Event make_event(const EventType& type, std::string source)
{
    return Event(type, std::move(source));
}

/**
 * 用来尝试从event.data中取出类型未Payload的payload
 * 取到就返回指针，取不到就返回nullptr
 */
template<typename Payload>
inline const Payload* event_data_as(const Event& event)
{
    return std::any_cast<Payload>(&event.data);
}

/**
 * 框架内置事件类型定义表
 */
namespace FrameworkEvents
{
// 纯EventType，空事件
inline const EventType NONE = make_event_type(EventCategory::NONE, "NONE");
// 关机事件
inline const EventType SHUTDOWN = make_event_type(EventCategory::RUNTIME, "SHUTDOWN");

// 通信事件：modbus接收
inline const TypedEvent<DeviceSample> MODBUS_SAMPLE_RECEIVED {
    make_event_type(EventCategory::COMMUNICATION, "MODBUS_SAMPLE_RECEIVED")
};

// 通信事件：websocket连接
inline const TypedEvent<WsClientInfo> WS_CLIENT_CONNECTED {
    make_event_type(EventCategory::COMMUNICATION, "WS_CLIENT_CONNECTED")
};

// 通信事件：websocket接收
inline const TypedEvent<WsMessage> WS_MESSAGE_RECEIVED {
    make_event_type(EventCategory::COMMUNICATION, "WS_MESSAGE_RECEIVED")
};

// 通信事件：websocket广播
inline const TypedEvent<DeviceSample> WS_BROADCAST {
    make_event_type(EventCategory::COMMUNICATION, "WS_BROADCAST")
};

// 通信事件：http client 请求
inline const TypedEvent<HttpClientRequest> HTTP_CLIENT_REQUESTED {
    make_event_type(EventCategory::COMMUNICATION, "HTTP_CLIENT_REQUESTED")
};

// 通信事件：http client 响应
inline const TypedEvent<HttpClientResponse> HTTP_CLIENT_RESPONSE_RECEIVED {
    make_event_type(EventCategory::COMMUNICATION, "HTTP_CLIENT_RESPONSE_RECEIVED")
};

// 通信事件：http client 失败
inline const TypedEvent<HttpClientFailure> HTTP_CLIENT_REQUEST_FAILED {
    make_event_type(EventCategory::COMMUNICATION, "HTTP_CLIENT_REQUEST_FAILED")
};
}

/**
 * 让EventType 可以被 unordered_map 快速索引，用来存和检查事件订阅者
 */
namespace std
{
template<>
struct hash<EventType>
{
    size_t operator()(const EventType& value) const noexcept
    {
        const size_t category_hash = std::hash<int> {}(static_cast<int>(value.category));
        const size_t name_hash = std::hash<std::string> {}(value.name);
        return category_hash ^ (name_hash << 1U);
    }
};
}
