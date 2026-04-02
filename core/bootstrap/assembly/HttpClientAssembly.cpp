#include "bootstrap/assembly/HttpClientAssembly.h"

#include <utility>

#include "bootstrap/factory/HttpClientBackendFactory.h"
#include "interfaces/http/HttpClientModule.h"

void HttpClientAssembly::install(AssemblyContext& context)
{
    if (!context.config.http_client.enabled)
    {
        context.http_client_module_factory = {};
        return;
    }

    const HttpClientConfig http_client_config = context.config.http_client;

    context.http_client_module_factory =
        [http_client_config](EventBus& bus) -> std::unique_ptr<HttpClientModule> {
            auto adapter = HttpClientBackendFactory::create(http_client_config);
            if (!adapter)
            {
                return nullptr;
            }

            return std::make_unique<HttpClientModule>(bus, std::move(adapter));
        };
}
