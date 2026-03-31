/**
 * HttpAssembly.h
 * 负责装配HTTP协议栈
 */
#pragma once

#include <functional>
#include <memory>

#include "bootstrap/assembly/AssemblyContext.h"
#include "interfaces/http/IHttpRouteProvider.h"
#include "runtime/bus/EventBus.h"

class HttpAssembly
{
public:
    using RouteProviderFactory = std::function<std::unique_ptr<IHttpRouteProvider>(EventBus&)>;

    static void install(
        AssemblyContext& context,
        RouteProviderFactory route_provider_factory = {});
};
