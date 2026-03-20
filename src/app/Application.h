#pragma once
#include <functional>
#include <memory>
#include <string>
#include "core/EventBus.h"
#include "core/EventLoop.h"
#include "interfaces/WebSocketGateway.h"
#include "modules/http/IHttpRouteProvider.h"
#include "modules/ModbusMasterModule.h"
#include "modules/HttpModule.h"
#include "services/ControlService.h"
#include "services/PracticeManager.h"
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
    void stop();
    void simulate_ws_client_connect(const std::string& client_id);
    void simulate_ws_client_message(const std::string& client_id, int addr, int value);
    void simulate_ws_text_message(const std::string& client_id, const std::string& text);

private:
    AppConfig config_;
    EventBus bus_;

    ModbusMasterModule modbus_;
    std::unique_ptr<HttpModule> http_;
    WebSocketGateway ws_;
    ControlService control_;
    PracticeManager practice_;
    EventLoop loop_;
};
