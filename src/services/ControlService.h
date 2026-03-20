#pragma once
#include "core/EventBus.h"
#include "core/Logger.h"

class ControlService {
public:
    explicit ControlService(EventBus& bus) : bus_(bus) {}

    void init() {
        bus_.subscribe(EventType::MODBUS_DATA_RECEIVED, [this](const Event& e) {
            on_modbus_data(e);
        });

        bus_.subscribe(EventType::WS_CLIENT_CONNECTED, [this](const Event& e) {
            on_ws_client_connected(e);
        });
    }

private:
    void on_modbus_data(const Event& e)
    {
        if (auto sample = std::get_if<DeviceSample>(&e.data))
        {
            Logger::info("[ControlService] processing device data: " + Logger::to_string(*sample));

            bus_.publish(Event(
                EventType::DATA_UPDATED,
                "ControlService",
                *sample
            ));
        }
        else
        {
            Logger::warn("[ControlService] MODBUS_DATA_RECEIVED data type mismatch");
        }
    }

    void on_ws_client_connected(const Event& e)
    {
        if (auto info = std::get_if<WsClientInfo>(&e.data))
        {
            Logger::info("[ControlService] WS client connected: " + Logger::to_string(*info));
        }
        else
        {
            Logger::warn("[ControlService] WS_CLIENT_CONNECTED data type mismatch");
        }
    }

private:
    EventBus& bus_;
};
