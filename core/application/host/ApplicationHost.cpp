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
    if (!destroyed_) {
        destroy();
    }
}

void ApplicationHost::init() {
    if (destroyed_) {
        Logger::error("[ApplicationHost] cannot init a destroyed host; create a new one via ApplicationBootstrap::create()");
        return;
    }
    if (initialized_) {
        Logger::warn("[ApplicationHost] already initialized, skipping duplicate init()");
        return;
    }

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

    initialized_ = true;
}

void ApplicationHost::run() {
    loop_.run();
}

void ApplicationHost::stop() {
    if (stopped_) {
        return;
    }

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

    stopped_ = true;
}

void ApplicationHost::destroy() {
    if (destroyed_) {
        return;
    }

    // 1. 停止事件循环和所有子模块
    stop();

    // 2. 释放所有业务模块
    business_modules_.clear();

    // 3. 释放定时器管理器，并清除 EventLoop 中的悬挂指针
    timer_manager_.reset();
    loop_.set_timer_manager(nullptr);

    // 4. 释放 HTTP 模块
    http_client_.reset();
    http_.reset();

    // 5. 释放 Modbus 模块
    modbus_master_.reset();
    modbus_device_runtimes_.clear();

    // 6. 清除 EventBus 所有订阅者，消除悬挂闭包（ws_ 等模块的订阅在此一并清理）
    bus_.clear();

    destroyed_ = true;
}

void ApplicationHost::simulate_ws_client_connect(const std::string& client_id) {
    ws_.simulate_client_connect(client_id);
}

void ApplicationHost::simulate_ws_client_message(const std::string& client_id, int addr, int value)
{
    ws_.simulate_client_message(client_id, addr, value);
}
