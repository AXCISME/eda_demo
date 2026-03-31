#pragma once

#include <functional>
#include <memory>

#include "bootstrap/AppConfig.h"
#include "application/host/ApplicationHost.h"
#include "interfaces/http/IHttpRouteProvider.h"
#include "runtime/bus/EventBus.h"

class ApplicationBootstrap {
public:
    using HttpRouteProviderFactory = std::function<std::unique_ptr<IHttpRouteProvider>(EventBus&)>;

    static std::unique_ptr<ApplicationHost> create(
        AppConfig config,
        HttpRouteProviderFactory http_route_provider_factory = {}
    );
};
