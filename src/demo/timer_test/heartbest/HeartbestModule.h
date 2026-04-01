#pragma once

#include "application/modules/IBusinessModule.h"
#include "demo/timer_test/TimerDemoEvents.h"
#include "runtime/logging/Logger.h"

class HeartbestModule : public IBusinessModule {
public:
    explicit HeartbestModule(EventBus& bus)
        : IBusinessModule(bus)
    {
    }

    void install() override {
        bus_.subscribe(TimerDemoEvents::TICK_1S, [this](const TimerDemoEvents::TimerTickPayload& payload) {
            send_heartbest(payload);
        });
    }

    void send_heartbest(const TimerDemoEvents::TimerTickPayload& payload) {
        Logger::info(
            "[HeartbestModule] heartbeat observed: "
            + payload.timer_name
            + " interval_ms=" + std::to_string(payload.interval_ms));
    }
};
