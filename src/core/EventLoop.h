#pragma once
#include "core/EventBus.h"
#include "core/Logger.h"

/**
 * EventLoop
 */
class EventLoop {
public:
    explicit EventLoop(EventBus& bus) : bus_(bus) {}

    void run() {
        running_ = true;
        Logger::info("[EventLoop] started");

        while (running_)
        {
            Event event;
            if (bus_.wait_and_get(event, 50))
            {
                if (event.type == EventType::SHUTDOWN)
                {
                    Logger::warn("[EventLoop] shutdown event received");
                    running_ = false;
                    break;
                }
                bus_.dispatch(event);
            }
            
        }
        Logger::info("[EventLoop] stopped");
    }

    void stop() {
        bus_.publish(Event(
            EventType::SHUTDOWN,
            "EventLoop"
        ));
    }
private:
    EventBus& bus_;
    bool running_ {false};
};
