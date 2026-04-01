#pragma once

#include "application/modules/IBusinessModule.h"
#include "demo/timer_test/TimerDemoEvents.h"
#include "runtime/logging/Logger.h"

class UpdateModule : public IBusinessModule {
public:
    explicit UpdateModule(EventBus& bus)
        : IBusinessModule(bus)
    {
    }

    void install() override {
        bus_.subscribe(TimerDemoEvents::TICK_5S, [this](const TimerDemoEvents::TimerTickPayload& payload) {
            update_data1(payload);
        });

        bus_.subscribe(TimerDemoEvents::TICK_10S, [this](const TimerDemoEvents::TimerTickPayload& payload) {
            update_data2(payload);
        });
    }

    void update_data1(const TimerDemoEvents::TimerTickPayload& payload) {
        Logger::info(
            "[UpdateModule] heartbeat observed: "
            + payload.timer_name
            + " interval_ms=" + std::to_string(payload.interval_ms));
    }

    void update_data2(const TimerDemoEvents::TimerTickPayload& payload) {
    Logger::info(
        "[UpdateModule] heartbeat observed: "
        + payload.timer_name
        + " interval_ms=" + std::to_string(payload.interval_ms));
    }
};
