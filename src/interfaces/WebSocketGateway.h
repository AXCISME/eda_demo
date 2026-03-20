#pragma once
#include <string>
#include <vector>
#include "core/EventBus.h"
#include "core/Logger.h"

class WebSocketGateway {
public:
    explicit WebSocketGateway(EventBus& bus) : bus_(bus) {}

    void init() {
        bus_.subscribe(EventType::DATA_UPDATED, [this](const Event& e) {
            on_data_updated(e);
        });

        bus_.subscribe(EventType::PRACTICE_STATE_CHANGED, [this](const Event& e) {
            on_practice_state_changed(e);
        });

        bus_.subscribe(EventType::WS_MESSAGE_RECEIVED, [this](const Event& e) {
            on_ws_message(e);
        });

        bus_.subscribe(EventType::WS_BROADCAST, [this](const Event& e) {
            on_broadcast(e);
        });
    }

    void simulate_client_connect(const std::string& client_id) {
        clients_.push_back(client_id);

        WsClientInfo info;
        info.client_id = client_id;

        bus_.publish(Event(
            EventType::WS_CLIENT_CONNECTED,
            "WebSocketGateway",
            info
        ));
    }

    void simulate_client_message(const std::string& client_id, int addr, int value) {
        WsMessage msg;
        msg.client_id = client_id;
        msg.text = "control";

        pending_command_.addr = addr;
        pending_command_.value = value;

        bus_.publish(Event(
            EventType::WS_MESSAGE_RECEIVED,
            "WebSocketGateway",
            msg
        ));
    }

    void simulate_text_message(const std::string& client_id, std::string text) {
        WsMessage msg;
        msg.client_id = client_id;
        msg.text = std::move(text);

        bus_.publish(Event(
            EventType::WS_MESSAGE_RECEIVED,
            "WebSocketGateway",
            msg
        ));
    }

private:
    void on_data_updated(const Event& e)
    {
        if (auto sample = std::get_if<DeviceSample>(&e.data))
        {
            bus_.publish(Event(
                EventType::WS_BROADCAST,
                "WebSocketGateway",
                *sample
            ));
        }
        else
        {
            Logger::warn("[WebSocketGateway] DATA_UPDATED data type mismatch");
        }
    }

    void on_ws_message(const Event& e)
    {
        auto msg = std::get_if<WsMessage>(&e.data);
        if (!msg)
        {
            Logger::warn("[WebSocketGateway] WS_MESSAGE_RECEIVED data type mismatch");
            return;
        }

        Logger::info("[WebSocketGateway] message received: " + Logger::to_string(*msg));

        if (msg->text == "control")
        {
            bus_.publish(Event(
                EventType::CONTROL_COMMAND,
                "WebSocketGateway",
                pending_command_
            ));
        }
        else if (msg->text == "practice:start" || msg->text == "practice:stop" || msg->text == "practice:reset")
        {
            PracticeCommand cmd;
            cmd.action = msg->text.substr(std::string("practice:").size());

            bus_.publish(Event(
                EventType::PRACTICE_COMMAND,
                "WebSocketGateway",
                cmd
            ));
        }
    }

    void on_broadcast(const Event& e)
    {
        for (const auto& client : clients_)
        {
            Logger::info("[WebSocketGateway] broadcast to [" + client + "] => " + Logger::to_string(e.data));
        }
    }

    void on_practice_state_changed(const Event& e)
    {
        auto state = std::get_if<PracticeState>(&e.data);
        if (!state)
        {
            Logger::warn("[WebSocketGateway] PRACTICE_STATE_CHANGED data type mismatch");
            return;
        }

        bus_.publish(Event(
            EventType::WS_BROADCAST,
            "WebSocketGateway",
            *state
        ));
    }

private:
    EventBus& bus_;
    std::vector<std::string> clients_;
    ControlCommand pending_command_;
};
