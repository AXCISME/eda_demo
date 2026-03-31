#pragma once

#include <functional>
#include <memory>
#include <string>

#include "bootstrap/AppConfig.h"
#include "runtime/bus/EventBus.h"
#include "runtime/loop/EventLoop.h"
#include "infrastructure/runtime/ModbusPollingRuntime.h"
#include "infrastructure/transport/modbus/IModbusMasterAdapter.h"
#include "interfaces/http/HttpModule.h"
#include "interfaces/modbus/ModbusMasterModule.h"
#include "interfaces/websocket/WebSocketModule.h"
#include "application/services/ControlService.h"

class ApplicationHost
{
public:
    using HttpModuleFactory = std::function<std::unique_ptr<HttpModule>(EventBus&)>;

    ApplicationHost(
        AppConfig config,
        std::unique_ptr<IModbusMasterAdapter> modbus_adapter,
        HttpModuleFactory http_module_factory = {});

    ~ApplicationHost();

    void init();
    void run();
    void stop();

    void simulate_ws_client_connect(const std::string& client_id);
    void simulate_ws_client_message(const std::string& client_id, int addr, int value);
private:
    AppConfig config_;
    EventBus bus_;

    std::unique_ptr<ModbusPollingRuntime> modbus_runtime_;
    std::unique_ptr<ModbusMasterModule> modbus_;

    std::unique_ptr<HttpModule> http_;
    WebSocketModule ws_;
    ControlService control_;
    EventLoop loop_;
};
