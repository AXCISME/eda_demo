#pragma once
#include <atomic>
#include <chrono>
#include <thread>
#include "core/EventBus.h"

class ModbusPollingRuntime
{
public:
    explicit ModbusPollingRuntime(EventBus& bus, int interval_ms = 1000)
        : bus_(bus),
          interval_ms_(interval_ms)
    {
    }

    ~ModbusPollingRuntime()
    {
        stop();
    }

    void start()
    {
        bool expected = false;
        if (!running_.compare_exchange_strong(expected, true))
        {
            return;
        }

        worker_ = std::thread([this]() {
            while (running_)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms_));
                if (!running_)
                {
                    break;
                }

                bus_.publish(Event(
                    EventType::MODBUS_POLL,
                    "ModbusPollingRuntime"
                ));
            }
        });
    }

    void stop()
    {
        bool expected = true;
        if (!running_.compare_exchange_strong(expected, false))
        {
            return;
        }

        if (worker_.joinable())
        {
            worker_.join();
        }
    }

private:
    EventBus& bus_;
    int interval_ms_ {1000};
    std::atomic<bool> running_ {false};
    std::thread worker_;
};
