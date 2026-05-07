#include "application/host/ApplicationHost.h"

#include <utility>

#include "runtime/logging/Logger.h"

ApplicationHost::ApplicationHost(
    AppConfig config,
    std::vector<std::unique_ptr<IModbusMasterAdapter>> modbus_master_adapters,
    HttpModuleFactory http_module_factory,
    HttpClientModuleFactory http_client_module_factory,
    TimerManagerFactory timer_manager_factory,
    BusinessModuleFactory business_module_factory)
    : config_(std::move(config)),
    ws_(bus_),
    loop_(bus_)
{
    if (config_.modbus_master.enabled)
    {
        const auto device_count = config_.modbus_master.devices.size();
        for (std::size_t index = 0;
             index < device_count && index < modbus_master_adapters.size();
             ++index)
        {
            modbus_device_runtimes_.push_back(std::make_unique<ModbusDeviceRuntime>(
                bus_,
                std::move(modbus_master_adapters[index]),
                config_.modbus_master.devices[index]));
        }

        if (!modbus_device_runtimes_.empty())
        {
            modbus_master_ = std::make_unique<ModbusMasterModule>(bus_, modbus_device_runtimes_);
        }
    }

    if (config_.http.enabled && http_module_factory)
    {
        http_ = http_module_factory(bus_);
    }

    if (config_.http_client.enabled && http_client_module_factory)
    {
        http_client_ = http_client_module_factory(bus_);
    }

    if (timer_manager_factory)
    {
        timer_manager_ = timer_manager_factory(bus_);
        loop_.set_timer_manager(timer_manager_.get());
    }

    if (business_module_factory)
    {
        business_modules_ = business_module_factory(bus_);
    }
}

ApplicationHost::~ApplicationHost() {
    stop();
}

void ApplicationHost::init() {
    if (modbus_master_)
    {
        modbus_master_->init();
    }
    else if (config_.modbus_master.enabled)
    {
        Logger::error("[ApplicationHost] Modbus Master is enabled but module was not constructed");
    }

    ws_.init();

    if (http_client_)
    {
        http_client_->init();
    }

    for (auto& module : business_modules_)
    {
        module->install();
    }

    if (modbus_master_ && !modbus_master_->start())
    {
        Logger::error("[ApplicationHost] failed to start Modbus Master runtime");
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
    if (http_client_)
    {
        http_client_->stop();
    }
    if (modbus_master_)
    {
        modbus_master_->stop();
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
