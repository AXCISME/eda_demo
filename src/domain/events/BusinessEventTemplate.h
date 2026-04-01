#pragma once

#include <string>

#include "domain/events/DomainEvent.h"

/*
 * 业务事件模板：
 * 1. 在你的业务模块中定义一个 payload 结构体。
 * 2. 为 EventPayloadFormatter<Payload> 提供特化，用于可读日志输出。
 * 3. 在你自己的命名空间中定义 TypedEvent<Payload> 事件常量。
 * 4. 发布事件：bus.publish(MyEvents::SOME_EVENT, "Source", payload);
 * 5. 订阅事件：bus.subscribe(MyEvents::SOME_EVENT, handler);
 *
 * handler 可以使用以下两种签名之一：
 *   [](const Payload& payload) { ... }
 *   [](const Event& event, const Payload& payload) { ... }
 */

namespace BusinessEventTemplate
{
struct ExamplePayload
{
    std::string message;
    int value {0};
};

inline const TypedEvent<ExamplePayload> EXAMPLE_EVENT {
    make_event_type(EventCategory::BUSINESS, "MY_MODULE.EXAMPLE_EVENT")
};
}

template<>
struct EventPayloadFormatter<BusinessEventTemplate::ExamplePayload>
{
    static std::string to_string(const BusinessEventTemplate::ExamplePayload& payload)
    {
        return "ExamplePayload{message=" + payload.message
            + ", value=" + std::to_string(payload.value)
            + "}";
    }
};
