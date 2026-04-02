#pragma once
#include <functional>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "domain/events/DomainEvent.h"
#include "runtime/bus/EventQueue.h"
#include "runtime/logging/Logger.h"

class EventBus {
public:
    using Handler = std::function<void(const Event&)>;

    void subscribe(const EventType& type, Handler handler) {
        subscribers_[type].push_back(std::move(handler));
    }

    template<typename Payload, typename TypedHandler>
    void subscribe(const TypedEvent<Payload>& definition, TypedHandler handler) {
        subscribe(definition.type, [handler = std::move(handler)](const Event& event) {
            const auto* payload = event_data_as<Payload>(event);
            if (!payload)
            {
                Logger::warn("[EventBus] payload type mismatch for " + to_string(event.type));
                return;
            }

            if constexpr (std::is_invocable_v<TypedHandler, const Event&, const Payload&>)
            {
                handler(event, *payload);
            }
            else if constexpr (std::is_invocable_v<TypedHandler, const Payload&>)
            {
                handler(*payload);
            }
            else
            {
                static_assert(
                    std::is_invocable_v<TypedHandler, const Event&, const Payload&>
                    || std::is_invocable_v<TypedHandler, const Payload&>,
                    "Typed event handlers must accept (const Event&, const Payload&) or (const Payload&).");
            }
        });
    }

    void publish(const Event& event) {
        Logger::info(
            std::string("[EventBus] publish -> ")
            + to_string(event.type)
            + " from=" + event.source
            + " payload=" + event.payload_text
        );
        queue_.push(event);
    }

    void publish(const EventType& type, std::string source) {
        publish(Event(type, std::move(source)));
    }

    /**
     * 这是模板函数，TypedEvent指定的Payload类型要和实际传入的payload类型强一致
     */
    template<typename Payload>
    void publish(const TypedEvent<Payload>& definition, std::string source, Payload payload) {
        publish(make_event(definition, std::move(source), std::move(payload)));
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
            + " subscriber_count=" + std::to_string(it->second.size())
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
