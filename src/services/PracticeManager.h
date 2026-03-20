#pragma once
#include <mutex>
#include "core/EventBus.h"
#include "core/Logger.h"

class PracticeManager {
public:
    explicit PracticeManager(EventBus& bus) : bus_(bus) {}

    void init() {
        bus_.subscribe(EventType::PRACTICE_COMMAND, [this](const Event& e) {
            on_practice_command(e);
        });

        bus_.subscribe(EventType::DATA_UPDATED, [this](const Event& e) {
            on_data_updated(e);
        });

        bus_.subscribe(EventType::WS_CLIENT_CONNECTED, [this](const Event& e) {
            on_ws_client_connected(e);
        });
    }

    PracticeState snapshot() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return state_;
    }

private:
    void on_practice_command(const Event& e)
    {
        auto cmd = std::get_if<PracticeCommand>(&e.data);
        if (!cmd)
        {
            Logger::warn("[PracticeManager] PRACTICE_COMMAND data type mismatch");
            return;
        }

        PracticeState next_state;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (cmd->action == "start")
            {
                state_.status = "running";
                state_.active = true;
                state_.sample_count = 0;
            }
            else if (cmd->action == "stop")
            {
                state_.status = "stopped";
                state_.active = false;
            }
            else if (cmd->action == "reset")
            {
                state_ = PracticeState{};
            }
            else
            {
                Logger::warn("[PracticeManager] unknown practice action: " + cmd->action);
                return;
            }

            next_state = state_;
        }

        publish_state("PracticeManager");
        Logger::info("[PracticeManager] state updated by command: " + Logger::to_string(next_state));
    }

    void on_data_updated(const Event& e)
    {
        auto sample = std::get_if<DeviceSample>(&e.data);
        if (!sample)
        {
            Logger::warn("[PracticeManager] DATA_UPDATED data type mismatch");
            return;
        }

        PracticeState next_state;
        bool should_publish = false;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!state_.active)
            {
                return;
            }

            state_.sample_count += 1;
            state_.last_device_id = sample->device_id;
            state_.last_temperature = sample->temperature;
            state_.last_pressure = sample->pressure;

            if (state_.sample_count >= completion_sample_target_)
            {
                state_.status = "completed";
                state_.active = false;
            }

            next_state = state_;
            should_publish = true;
        }

        if (should_publish)
        {
            publish_state("PracticeManager");
            Logger::info("[PracticeManager] state advanced by sample: " + Logger::to_string(next_state));
        }
    }

    void on_ws_client_connected(const Event& e)
    {
        auto info = std::get_if<WsClientInfo>(&e.data);
        if (!info)
        {
            Logger::warn("[PracticeManager] WS_CLIENT_CONNECTED data type mismatch");
            return;
        }

        Logger::info("[PracticeManager] syncing state to client: " + info->client_id);
        publish_state("PracticeManager");
    }

    void publish_state(const std::string& source)
    {
        bus_.publish(Event(
            EventType::PRACTICE_STATE_CHANGED,
            source,
            snapshot()
        ));
    }

private:
    EventBus& bus_;
    mutable std::mutex mutex_;
    PracticeState state_;
    int completion_sample_target_ {3};
};
