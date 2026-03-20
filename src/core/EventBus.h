#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include "core/Event.h"
#include "core/EventQueue.h"
#include "core/Logger.h"

class EventBus {
public:
    using Handler = std::function<void(const Event&)>;

    void subscribe(EventType type, Handler handler) {
        subscribers_[type].push_back(std::move(handler));
    }

    void publish(const Event& event) {
        Logger::info(
            std::string("[EventBus] publish -> ")
            + to_string(event.type)
            + " from=" + event.source
            + " payload=" + Logger::to_string(event.data)
        );
        queue_.push(event);
    }

    bool wait_and_get(Event& event, int timeout_ms) {
        return queue_.wait_and_pop(event, timeout_ms);
    }

    void dispatch(const Event& event) {
        auto it = subscribers_.find(event.type);
        if (it == subscribers_.end())
        {
            Logger::warn(std::string("[EventBus] no subsriber for ") + to_string(event.type));
            return;
        }
        
        Logger::info(
            std::string("[EventBus] dispatch -> ")
            + to_string(event.type)
            + "subscriber_count=" + std::to_string(it->second.size())
        );

        for (auto& handler : it->second)
        {
            handler(event);
        }
    }

private:
    std::unordered_map<EventType, std::vector<Handler>> subscribers_;
    EventQueue queue_;
};
