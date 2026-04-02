#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "application/modules/IBusinessModule.h"
#include "bootstrap/AppConfig.h"
#include "runtime/bus/EventBus.h"
#include "runtime/loop/EventLoop.h"
#include "runtime/scheduler/TimerManager.h"
#include "infrastructure/runtime/ModbusPollingRuntime.h"
#include "infrastructure/transport/modbus/IModbusMasterAdapter.h"
#include "interfaces/http/HttpClientModule.h"
#include "interfaces/http/HttpModule.h"
#include "interfaces/modbus/ModbusMasterModule.h"
#include "interfaces/websocket/WebSocketModule.h"

class ApplicationHost
{
public:
    using HttpModuleFactory = std::function<std::unique_ptr<HttpModule>(EventBus&)>;
    using HttpClientModuleFactory = std::function<std::unique_ptr<HttpClientModule>(EventBus&)>;
    using TimerManagerFactory = std::function<std::unique_ptr<TimerManager>(EventBus&)>;
    using BusinessModuleFactory = std::function<std::vector<std::unique_ptr<IBusinessModule>>(EventBus&)>;

    ApplicationHost(
        AppConfig config,
        std::unique_ptr<IModbusMasterAdapter> modbus_master_adapter,
        HttpModuleFactory http_module_factory = {},
        HttpClientModuleFactory http_client_module_factory = {},
        TimerManagerFactory timer_manager_factory = {},
        BusinessModuleFactory business_module_factory = {});

    ~ApplicationHost();

    void init();
    void run();
    void stop();

    void simulate_ws_client_connect(const std::string& client_id);
    void simulate_ws_client_message(const std::string& client_id, int addr, int value);
private:
    AppConfig config_;
    EventBus bus_;

    std::unique_ptr<ModbusPollingRuntime> modbus_master_runtime_;
    std::unique_ptr<ModbusMasterModule> modbus_master_;

    std::unique_ptr<HttpModule> http_;
    std::unique_ptr<HttpClientModule> http_client_;
    std::unique_ptr<TimerManager> timer_manager_;
    std::vector<std::unique_ptr<IBusinessModule>> business_modules_;
    WebSocketModule ws_;
    EventLoop loop_;
};
