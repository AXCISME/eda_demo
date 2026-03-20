#pragma once
#include <functional>
#include <memory>
#include "core/EventBus.h"
#include "core/EventLoop.h"
#include "core/TimerManager.h"
#include "modules/http/IHttpRouteProvider.h"
#include "modules/ModbusMasterModule.h"
#include "modules/HttpModule.h"
#include "modules/WebSocketModule.h"
#include "services/ControlService.h"
#include "app/AppConfig.h"

class Application
{
public:
    using HttpRouteProviderFactory = std::function<std::unique_ptr<IHttpRouteProvider>(EventBus&)>;

    explicit Application(
        AppConfig config = AppConfig{},
        HttpRouteProviderFactory http_route_provider_factory = {});

    void init();
    void run();

private:
    AppConfig config_;
    EventBus bus_;
    TimerManager timer_manager_;

    ModbusMasterModule modbus_;
    std::unique_ptr<HttpModule> http_;
    WebSocketModule ws_;
    ControlService control_;
    EventLoop loop_;
};
