#pragma once
#include <memory>
#include <vector>
#include "core/EventBus.h"
#include "core/Logger.h"
#include "transport/modbus/IModbusMasterAdapter.h"

class ModbusMasterModule
{
public:
    ModbusMasterModule(EventBus& bus, std::unique_ptr<IModbusMasterAdapter> adapter)
        : bus_(bus),
          adapter_(std::move(adapter))
    {
    }

    void init()
    {
        bus_.subscribe(EventType::MODBUS_POLL, [this](const Event& e) {
            on_poll(e);
        });

        bus_.subscribe(EventType::CONTROL_COMMAND, [this](const Event& e) {
            on_control_command(e);
        });

        bus_.subscribe(EventType::MODBUS_WRITE_REQUEST, [this](const Event& e) {
            on_write_request(e);
        });
    }

    bool start()
    {
        if (!adapter_)
        {
            Logger::error("[ModbusMasterModule] adapter is null");
            return false;
        }

        return adapter_->connect();
    }

    void stop()
    {
        if (adapter_)
        {
            adapter_->disconnect();
        }
    }

private:
    void on_poll(const Event&)
    {
        if (!adapter_)
        {
            Logger::error("[ModbusMasterModule] adapter is null");
            return;
        }

        std::vector<uint16_t> regs;
        if (!adapter_->read_holding_registers(default_slave_id_, sample_base_addr_, 2, regs))
        {
            Logger::error("[ModbusMasterModule] failed to read holding registers");
            return;
        }

        if (regs.size() < 2)
        {
            Logger::error("[ModbusMasterModule] insufficient register data");
            return;
        }

        DeviceSample sample;
        sample.device_id = "dev-" + std::to_string(default_slave_id_);
        sample.temperature = static_cast<float>(regs[0]) / 10.0f;
        sample.pressure = static_cast<float>(regs[1]) / 10.0f;

        Logger::info("[ModbusMasterModule] sample read: " + Logger::to_string(sample));

        bus_.publish(Event(
            EventType::MODBUS_DATA_RECEIVED,
            "ModbusMasterModule",
            sample
        ));
    }

    void on_control_command(const Event& e)
    {
        if (auto cmd = std::get_if<ControlCommand>(&e.data))
        {
            Logger::info("[ModbusMasterModule] received control command: " + Logger::to_string(*cmd));

            bus_.publish(Event(
                EventType::MODBUS_WRITE_REQUEST,
                "ModbusMasterModule",
                *cmd
            ));
        }
        else
        {
            Logger::warn("[ModbusMasterModule] CONTROL_COMMAND data type mismatch");
        }
    }

    void on_write_request(const Event& e)
    {
        if (!adapter_)
        {
            Logger::error("[ModbusMasterModule] adapter is null");
            return;
        }

        auto cmd = std::get_if<ControlCommand>(&e.data);
        if (!cmd)
        {
            Logger::warn("[ModbusMasterModule] MODBUS_WRITE_REQUEST data type mismatch");
            return;
        }

        if (!adapter_->write_single_register(
                default_slave_id_,
                cmd->addr,
                static_cast<uint16_t>(cmd->value)))
        {
            Logger::error("[ModbusMasterModule] write single register failed");
        }
    }

private:
    EventBus& bus_;
    std::unique_ptr<IModbusMasterAdapter> adapter_;
    int default_slave_id_ {1};
    int sample_base_addr_ {100};
};
