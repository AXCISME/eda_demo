#pragma once
#include <string>
#include <vector>
#include "runtime/bus/EventBus.h"
#include "runtime/logging/Logger.h"

class WebSocketModule {
public:
    explicit WebSocketModule(EventBus& bus) : bus_(bus) {}

    void init() {
        bus_.subscribe(EventType::TELEMETRY_UPDATED, [this](const Event& e) {
            on_telemetry_updated(e);
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
            "WebSocketModule",
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
            "WebSocketModule",
            msg
        ));
    }

private:
    void on_telemetry_updated(const Event& e)
    {
        if (auto sample = std::get_if<DeviceSample>(&e.data))
        {
            bus_.publish(Event(
                EventType::WS_BROADCAST,
                "WebSocketModule",
                *sample
            ));
        }
        else
        {
            Logger::warn("[WebSocketModule] DATA_UPDATED data type mismatch");
        }
    }

    void on_ws_message(const Event& e)
    {
        auto msg = std::get_if<WsMessage>(&e.data);
        if (!msg)
        {
            Logger::warn("[WebSocketModule] WS_MESSAGE_RECEIVED data type mismatch");
            return;
        }

        Logger::info("[WebSocketModule] message received: " + Logger::to_string(*msg));

        if (msg->text == "control")
        {
            bus_.publish(Event(
                EventType::CONTROL_COMMAND,
                "WebSocketModule",
                pending_command_
            ));
        }
    }

    void on_broadcast(const Event& e)
    {
        for (const auto& client : clients_)
        {
            Logger::info("[WebSocketModule] broadcast to [" + client + "] => " + Logger::to_string(e.data));
        }
    }
private:
    EventBus& bus_;
    std::vector<std::string> clients_;
    ControlCommand pending_command_;
};
