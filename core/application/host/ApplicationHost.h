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
#include "infrastructure/runtime/ModbusDeviceRuntime.h"
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
        std::vector<std::unique_ptr<IModbusMasterAdapter>> modbus_master_adapters,
        HttpModuleFactory http_module_factory = {},
        HttpClientModuleFactory http_client_module_factory = {},
        TimerManagerFactory timer_manager_factory = {},
        BusinessModuleFactory business_module_factory = {});

    ~ApplicationHost();

    /**
     * 初始化所有子系统。必须在 create() 之后、run() 之前调用。
     * 只能调用一次：重复调用会被忽略，destroy() 之后调用会报错。
     */
    void init();

    /**
     * 运行事件循环（阻塞直到 stop() 被调用）。
     */
    void run();

    /**
     * 停止事件循环和所有子模块。可重复调用（幂等）。
     * 调用后可通过 run() 重新进入事件循环。
     */
    void stop();

    /**
     * 终止性销毁：释放所有资源，清除 EventBus 订阅者，使 host 不可再用。
     * 调用后必须通过 ApplicationBootstrap::create() 创建新实例，不能对同一对象 re-init。
     */
    void destroy();

    void simulate_ws_client_connect(const std::string& client_id);
    void simulate_ws_client_message(const std::string& client_id, int addr, int value);
private:
    AppConfig config_;
    EventBus bus_;

    std::vector<std::unique_ptr<ModbusDeviceRuntime>> modbus_device_runtimes_;
    std::unique_ptr<ModbusMasterModule> modbus_master_;

    std::unique_ptr<HttpModule> http_;
    std::unique_ptr<HttpClientModule> http_client_;
    std::unique_ptr<TimerManager> timer_manager_;
    std::vector<std::unique_ptr<IBusinessModule>> business_modules_;
    WebSocketModule ws_;
    EventLoop loop_;

    bool initialized_{false};
    bool stopped_{false};
    bool destroyed_{false};
};
