#pragma once

#include "application/events/ApplicationEvents.h"
#include "infrastructure/runtime/ModbusPollingRuntime.h"
#include "runtime/bus/EventBus.h"
#include "runtime/logging/Logger.h"

class ModbusMasterModule
{
public:
    ModbusMasterModule(EventBus& bus, ModbusPollingRuntime& runtime)
        : bus_(bus),
          runtime_(runtime)
    {
    }

    void init()
    {
        bus_.subscribe(ApplicationEvents::CONTROL_COMMAND, [this](const ControlCommand& cmd) {
            on_control_command(cmd);
        });
    }

    bool start()
    {
        return runtime_.start();
    }

    void stop()
    {
        runtime_.stop();
    }
private:
    void on_control_command(const ControlCommand& cmd)
    {
        Logger::info("[ModbusMasterModule] forward control command: " + Logger::to_string(cmd));
        runtime_.submit_write(cmd);
    }
    
private:
    EventBus& bus_;
    ModbusPollingRuntime& runtime_;
};
