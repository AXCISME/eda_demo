#pragma once
#include <memory>
#include <utility>
#include "core/Logger.h"
#include "modules/http/BaseHttpRouteProvider.h"
#include "modules/http/IHttpRouteProvider.h"
#include "modules/http/IHttpRouteRegistry.h"
#include "transport/http/IHttpServerAdapter.h"

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
