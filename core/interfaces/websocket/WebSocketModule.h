#pragma once
#include <string>
#include <vector>

#include "application/events/ApplicationEvents.h"
#include "runtime/bus/EventBus.h"
#include "runtime/logging/Logger.h"

class WebSocketModule {
public:
    explicit WebSocketModule(EventBus& bus) : bus_(bus) {}

    void init() {
        bus_.subscribe(ApplicationEvents::TELEMETRY_UPDATED, [this](const DeviceSample& sample) {
            on_telemetry_updated(sample);
        });

        bus_.subscribe(FrameworkEvents::WS_MESSAGE_RECEIVED, [this](const WsMessage& msg) {
            on_ws_message(msg);
        });

        bus_.subscribe(FrameworkEvents::WS_BROADCAST, [this](const DeviceSample& sample) {
            on_broadcast(sample);
        });
    }

    void simulate_client_connect(const std::string& client_id) {
        clients_.push_back(client_id);

        WsClientInfo info;
        info.client_id = client_id;

        bus_.publish(FrameworkEvents::WS_CLIENT_CONNECTED, "WebSocketModule", info);
    }

    void simulate_client_message(const std::string& client_id, int addr, int value) {
        WsMessage msg;
        msg.client_id = client_id;
        msg.text = "control";

        pending_command_.addr = addr;
        pending_command_.value = value;

        bus_.publish(FrameworkEvents::WS_MESSAGE_RECEIVED, "WebSocketModule", msg);
    }

private:
    void on_telemetry_updated(const DeviceSample& sample)
    {
        bus_.publish(FrameworkEvents::WS_BROADCAST, "WebSocketModule", sample);
    }

    void on_ws_message(const WsMessage& msg)
    {
        Logger::info("[WebSocketModule] message received: " + Logger::to_string(msg));

        if (msg.text == "control")
        {
            bus_.publish(ApplicationEvents::CONTROL_COMMAND, "WebSocketModule", pending_command_);
        }
    }

    void on_broadcast(const DeviceSample& sample)
    {
        for (const auto& client : clients_)
        {
            Logger::info("[WebSocketModule] broadcast to [" + client + "] => " + Logger::to_string(sample));
        }
    }
private:
    EventBus& bus_;
    std::vector<std::string> clients_;
    ControlCommand pending_command_;
};
