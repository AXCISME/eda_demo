/**
 * HttpModule.h
 * 把 HTTP 适配器和路由器组装起来
 * 需要提供IHttpServerAdapter的实现，实现具体的HTTP服务器
 * 需要提供IHttpRouteProvider的实现，实现具体的路由
 */
#pragma once
#include <memory>
#include <utility>
#include "runtime/logging/Logger.h"
#include "interfaces/http/BaseHttpRouteProvider.h"
#include "interfaces/http/IHttpRouteProvider.h"
#include "interfaces/http/IHttpRouteRegistry.h"
#include "infrastructure/transport/http/IHttpServerAdapter.h"

class EventBus;

class HttpModule
{
public:
    HttpModule(
        std::unique_ptr<IHttpServerAdapter> adapter,
        std::unique_ptr<IHttpRouteProvider> route_provider)
        : adapter_(std::move(adapter)),
          route_provider_(std::move(route_provider))
    {
    }

    void init()
    {
        if (!adapter_)
        {
            Logger::error("[HttpModule] adapter is null");
            return;
        }

        if (!route_provider_)
        {
            Logger::error("[HttpModule] route_provider is null");
            return;
        }

        AdapterRouteRegistry registry(*adapter_);
        route_provider_->register_routes(registry);
    }

    bool start(const std::string& host, int port)
    {
        if (!adapter_)
        {
            Logger::error("[HttpModule] adapter is null");
            return false;
        }

        return adapter_->start(host, port);
    }

    void stop()
    {
        if (adapter_)
        {
            adapter_->stop();
        }
    }

private:
    class AdapterRouteRegistry : public IHttpRouteRegistry
    {
    public:
        explicit AdapterRouteRegistry(IHttpServerAdapter& adapter)
            : adapter_(adapter)
        {
        }

        void add_route(
            const std::string& method,
            const std::string& path,
            Handler handler) override
        {
            adapter_.register_handler(method, path, std::move(handler));
        }

    private:
        IHttpServerAdapter& adapter_;
    };

private:
    std::unique_ptr<IHttpServerAdapter> adapter_;
    std::unique_ptr<IHttpRouteProvider> route_provider_;
};
