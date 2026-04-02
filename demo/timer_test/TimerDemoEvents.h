#pragma once

#include <string>

#include "domain/events/DomainEvent.h"

namespace TimerDemoEvents
{

/**
 * Payload 结构体
 */
struct TimerTickPayload
{
    std::string timer_name;
    int interval_ms {0};
};

/**
 * 定义包含 TimerTickPayload 的事件
 */
inline const TypedEvent<TimerTickPayload> TICK_1S {
    make_event_type(EventCategory::BUSINESS, "TIMER_DEMO.TICK_1S")
};

inline const TypedEvent<TimerTickPayload> TICK_5S {
    make_event_type(EventCategory::BUSINESS, "TIMER_DEMO.TICK_5S")
};

inline const TypedEvent<TimerTickPayload> TICK_10S {
    make_event_type(EventCategory::BUSINESS, "TIMER_DEMO.TICK_10S")
};
}

template<>
struct EventPayloadFormatter<TimerDemoEvents::TimerTickPayload>
{
    static std::string to_string(const TimerDemoEvents::TimerTickPayload& payload)
    {
        return "TimerTickPayload{timer_name=" + payload.timer_name
            + ", interval_ms=" + std::to_string(payload.interval_ms)
            + "}";
    }
};
