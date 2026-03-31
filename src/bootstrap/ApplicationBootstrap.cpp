#include "bootstrap/ApplicationBootstrap.h"

#include <memory>
#include <utility>

#include "runtime/logging/Logger.h"
#include "interfaces/http/BaseHttpRouteProvider.h"
#include "interfaces/http/HttpModule.h"
#include "infrastructure/transport/http/IHttpServerAdapter.h"
#include "infrastructure/transport/http/FakeHttpServerAdapter.h"
#include "infrastructure/transport/modbus/FakeModbusMasterAdapter.h"
#if EDA_ENABLE_HTTP
#include "infrastructure/transport/http/MongooseHttpServerAdapter.h"
#endif

namespace
{
std::unique_ptr<IModbusMasterAdapter> create_modbus_adapter()
{
    return std::make_unique<FakeModbusMasterAdapter>();
}

std::unique_ptr<IHttpServerAdapter> create_http_adapter(const AppConfig& config)
{
    if (config.http_backend == "fake")
    {
        return std::make_unique<FakeHttpServerAdapter>();
    }

    if (config.http_backend == "mongoose")
    {
#if EDA_ENABLE_HTTP
        return std::make_unique<MongooseHttpServerAdapter>();
#else
        Logger::warn("[ApplicationBootstrap] mongoose backend unavailable because EDA_ENABLE_HTTP=0");
        return nullptr;
#endif
    }

    Logger::error("[ApplicationBootstrap] unknown http backend: " + config.http_backend);
    return nullptr;
}
}

std::unique_ptr<ApplicationHost> ApplicationBootstrap::create(
    AppConfig config,
    HttpRouteProviderFactory http_route_provider_factory)
{
    ApplicationHost::HttpModuleFactory http_module_factory;

    if (config.enable_http)
    {
        const AppConfig http_config = config;

        http_module_factory =
            [http_config, http_route_provider_factory](EventBus& bus) -> std::unique_ptr<HttpModule> {
                auto adapter = create_http_adapter(http_config);
                if (!adapter)
                {
                    return nullptr;
                }

                auto route_provider = http_route_provider_factory
                    ? http_route_provider_factory(bus)
                    : std::make_unique<BaseHttpRouteProvider>();

                return std::make_unique<HttpModule>(
                    std::move(adapter),
                    std::move(route_provider));
            };
    }

    return std::make_unique<ApplicationHost>(
        std::move(config),
        create_modbus_adapter(),
        std::move(http_module_factory));
}
