#include "application/host/ApplicationHost.h"

#include <utility>

#include "runtime/logging/Logger.h"

ApplicationHost::ApplicationHost(
    AppConfig config,
    std::unique_ptr<IModbusMasterAdapter> modbus_adapter,
    HttpModuleFactory http_module_factory)
    : config_(std::move(config)),
    modbus_runtime_(bus_,
        std::move(modbus_adapter),
        config_.modbus_poll_interval_ms),
    modbus_(bus_, modbus_runtime_),
    ws_(bus_),
    control_(bus_),
    loop_(bus_)
{
    if (config_.enable_http && http_module_factory)
    {
        http_ = http_module_factory(bus_);
    }
}

ApplicationHost::~ApplicationHost() {
    stop();
}

void ApplicationHost::init() {
    modbus_.init();
    ws_.init();
    control_.init();

    if (!modbus_.start())
    {
        Logger::error("[ApplicationHost] failed to start Modbus runtime");
    }
    
    if (http_)
    {
        http_->init();
        if (!http_->start(config_.http_host, config_.http_port))
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
    modbus_.stop();
    loop_.stop();
}

void ApplicationHost::simulate_ws_client_connect(const std::string& client_id) {
    ws_.simulate_client_connect(client_id);
}

void ApplicationHost::simulate_ws_client_message(const std::string& client_id, int addr, int value)
{
    ws_.simulate_client_message(client_id, addr, value);
}
