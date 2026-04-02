#include "bootstrap/assembly/HttpAssembly.h"

#include <utility>

#include "bootstrap/factory/HttpBackendFactory.h"
#include "interfaces/http/BaseHttpRouteProvider.h"
#include "interfaces/http/HttpModule.h"

void HttpAssembly::install(
    AssemblyContext& context,
    RouteProviderFactory route_provider_factory)
{
    if (!context.config.http.enabled)
    {
        context.http_module_factory = {};
        return;
    }

    const HttpConfig http_config = context.config.http;

    context.http_module_factory =
        [http_config, route_provider_factory](EventBus& bus) -> std::unique_ptr<HttpModule> {
            auto adapter = HttpBackendFactory::create(http_config);
            if (!adapter)
            {
                return nullptr;
            }

            auto route_provider = route_provider_factory
                ? route_provider_factory(bus)
                : std::make_unique<BaseHttpRouteProvider>();

            return std::make_unique<HttpModule>(
                std::move(adapter),
                std::move(route_provider));
        };
}
