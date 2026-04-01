#pragma once

#include "application/events/ApplicationEvents.h"
#include "application/modules/IBusinessModule.h"
#include "runtime/logging/Logger.h"

class ControlService : public IBusinessModule
{
public:
    explicit ControlService(EventBus& bus)
        : IBusinessModule(bus)
    {
    }

    void install() override
    {
        bus_.subscribe(FrameworkEvents::MODBUS_SAMPLE_RECEIVED, [this](const DeviceSample& sample) {
            on_modbus_sample_received(sample);
        });

        bus_.subscribe(FrameworkEvents::WS_CLIENT_CONNECTED, [this](const WsClientInfo& info) {
            on_ws_client_connected(info);
        });
    }

private:
    void on_modbus_sample_received(const DeviceSample& sample)
    {
        Logger::info(
            "[ControlService] processing telemetry sample: "
            + Logger::to_string(sample));

        bus_.publish(ApplicationEvents::TELEMETRY_UPDATED, "ControlService", sample);
    }

    void on_ws_client_connected(const WsClientInfo& info)
    {
        Logger::info("[ControlService] WS client connected: " + Logger::to_string(info));
    }

};
