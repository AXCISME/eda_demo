#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "infrastructure/runtime/ModbusDeviceRuntime.h"
#include "runtime/bus/EventBus.h"
#include "runtime/logging/Logger.h"

class ModbusMasterModule
{
public:
    ModbusMasterModule(EventBus& bus, const std::vector<std::unique_ptr<ModbusDeviceRuntime>>& runtimes)
        : bus_(bus)
    {
        for (const auto& runtime : runtimes)
        {
            if (runtime)
            {
                runtimes_[runtime->device_name()] = runtime.get();
            }
        }
    }

    void init()
    {
        bus_.subscribe(FrameworkEvents::READ_REQUEST, [this](const ModbusReadRequest& req) {
            on_read_request(req);
        });

        bus_.subscribe(FrameworkEvents::WRITE_REQUEST, [this](const ModbusWriteRequest& req) {
            on_write_request(req);
        });
    }

    bool start()
    {
        bool ok = true;
        for (auto& [device_name, runtime] : runtimes_)
        {
            if (!runtime->start())
            {
                Logger::error("[ModbusMasterModule] failed to start device=" + device_name);
                ok = false;
            }
        }
        return ok;
    }

    void stop()
    {
        for (auto& [device_name, runtime] : runtimes_)
        {
            runtime->stop();
        }
    }
private:
    void on_read_request(const ModbusReadRequest& req)
    {
        auto* runtime = find_runtime(req.device_name);
        if (!runtime)
        {
            publish_route_failure(req.request_id, req.device_name);
            return;
        }

        runtime->submit_read(req);
    }

    void on_write_request(const ModbusWriteRequest& req)
    {
        auto* runtime = find_runtime(req.device_name);
        if (!runtime)
        {
            publish_route_failure(req.request_id, req.device_name);
            return;
        }

        runtime->submit_write(req);
    }

    ModbusDeviceRuntime* find_runtime(const std::string& device_name) const
    {
        auto it = runtimes_.find(device_name);
        if (it == runtimes_.end())
        {
            return nullptr;
        }

        return it->second;
    }

    void publish_route_failure(const std::string& request_id, const std::string& device_name)
    {
        Logger::error("[ModbusMasterModule] unknown modbus device=" + device_name);

        ModbusOperationFailed failure;
        failure.request_id = request_id;
        failure.device_name = device_name;
        failure.error_message = "unknown modbus device";
        bus_.publish(FrameworkEvents::OP_FAILED, "ModbusMasterModule", std::move(failure));
    }
    
private:
    EventBus& bus_;
    std::unordered_map<std::string, ModbusDeviceRuntime*> runtimes_;
};
