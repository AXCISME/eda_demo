#include "application/host/ApplicationHost.h"

#include <utility>

#include "runtime/logging/Logger.h"

ApplicationHost::ApplicationHost(
    AppConfig config,
    std::unique_ptr<IModbusMasterAdapter> modbus_adapter,
    HttpModuleFactory http_module_factory)
    : config_(std::move(config)),
    ws_(bus_),
    control_(bus_),
    loop_(bus_)
{
    if (config_.modbus.enabled && modbus_adapter)
    {
        modbus_runtime_ = std::make_unique<ModbusPollingRuntime>(
            bus_,
            std::move(modbus_adapter),
            config_.modbus.poll_interval_ms,
            config_.modbus.slave_id,
            config_.modbus.sample_base_addr);
        modbus_ = std::make_unique<ModbusMasterModule>(bus_, *modbus_runtime_);
    }

    if (config_.http.enabled && http_module_factory)
    {
        http_ = http_module_factory(bus_);
    }
}

ApplicationHost::~ApplicationHost() {
    stop();
}

void ApplicationHost::init() {
    if (modbus_)
    {
        modbus_->init();
    }
    else if (config_.modbus.enabled)
    {
        Logger::error("[ApplicationHost] Modbus is enabled but module was not constructed");
    }

    ws_.init();
    control_.init();

    if (modbus_ && !modbus_->start())
    {
        Logger::error("[ApplicationHost] failed to start Modbus runtime");
    }
    
    if (http_)
    {
        http_->init();
        if (!http_->start(config_.http.host, config_.http.port))
        {
            Logger::error("[ApplicationHost] failed to start HTTP server");
        }
    }
}

void ApplicationHost::run() {
    loop_.run();
}

void ApplicationHost::stop() {
    if (http_)
    {
        http_->stop();
    }
    if (modbus_)
    {
        modbus_->stop();
    }
    loop_.stop();
}

void ApplicationHost::simulate_ws_client_connect(const std::string& client_id) {
    ws_.simulate_client_connect(client_id);
}

void ApplicationHost::simulate_ws_client_message(const std::string& client_id, int addr, int value)
{
    ws_.simulate_client_message(client_id, addr, value);
}
