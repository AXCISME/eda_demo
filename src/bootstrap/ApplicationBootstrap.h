/**
 * ApplicationBootstrap.h
 * 真正的总装入口
 * 1. 接收AppConfig
 * 2. 校验配置
 * 3. 调用各协议的 assembly
 * 4. 用 assembly 产物去构造 ApplicationHost
 */
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
