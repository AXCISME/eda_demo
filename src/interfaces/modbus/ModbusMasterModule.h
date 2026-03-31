#pragma once

#include "runtime/bus/EventBus.h"
#include "runtime/logging/Logger.h"
#include "infrastructure/runtime/ModbusPollingRuntime.h"

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
        bus_.subscribe(EventType::CONTROL_COMMAND, [this](const Event& e) {
            on_control_command(e);
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
    void on_control_command(const Event& e)
    {
        auto cmd = std::get_if<ControlCommand>(&e.data);
        if (!cmd)
        {
            Logger::warn("[ModbusMasterModule] CONTROL_COMMAND data type mismatch");
            return;
        }

        Logger::info("[ModbusMasterModule] forward control command: " + Logger::to_string(*cmd));
        runtime_.submit_write(*cmd);
    }
    
private:
    EventBus& bus_;
    ModbusPollingRuntime& runtime_;
};
