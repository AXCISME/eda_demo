#pragma once

#include <string>

#include "application/modules/IBusinessModule.h"
#include "domain/events/DomainEvent.h"
#include "runtime/logging/Logger.h"

class ModbusPollingModule : public IBusinessModule
{
public:
    explicit ModbusPollingModule(EventBus& bus, std::string device_name)
        : IBusinessModule(bus),
          device_name_(std::move(device_name))
    {
    }

    void install() override
    {
        bus_.subscribe(FrameworkEvents::READ_RESULT, [this](const ModbusReadResult& result) {
            on_read_result(result);
        });

        bus_.subscribe(FrameworkEvents::OP_FAILED, [this](const ModbusOperationFailed& failure) {
            on_op_failed(failure);
        });
    }

private:
    void on_read_result(const ModbusReadResult& result)
    {
        if (result.device_name != device_name_)
        {
            return;
        }

        std::string detail;
        for (std::size_t i = 0; i < result.values.size(); ++i)
        {
            if (i > 0)
            {
                detail += ", ";
            }
            detail += "addr[" + std::to_string(result.addr + static_cast<int>(i))
                    + "]=" + std::to_string(result.values[i]);
        }

        Logger::info(
            "[ModbusPollingModule][" + device_name_ + "] read result: "
            + to_string(result.reg_type)
            + " slave=" + std::to_string(result.slave_id)
            + " " + detail);
    }

    void on_op_failed(const ModbusOperationFailed& failure)
    {
        if (failure.device_name != device_name_)
        {
            return;
        }

        Logger::error(
            "[ModbusPollingModule][" + device_name_ + "] operation failed: "
            + failure.error_message);
    }

private:
    std::string device_name_;
};
