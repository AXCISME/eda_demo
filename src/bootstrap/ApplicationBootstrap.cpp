#include "bootstrap/ApplicationBootstrap.h"

#include <memory>
#include <utility>
#include <vector>

#include "bootstrap/AppConfigValidation.h"
#include "bootstrap/assembly/AssemblyContext.h"
#include "bootstrap/assembly/HttpAssembly.h"
#include "bootstrap/assembly/ModbusMasterAssembly.h"
#include "runtime/logging/Logger.h"

namespace
{
void log_validation_errors(const std::vector<std::string>& errors)
{
    for (const auto& error : errors)
    {
        Logger::error("[ApplicationBootstrap] invalid config: " + error);
    }
}
}

std::unique_ptr<ApplicationHost> ApplicationBootstrap::create(
    AppConfig config,
    HttpRouteProviderFactory http_route_provider_factory)
{
    AssemblyContext context(std::move(config));

    const auto validation_errors = validate_app_config(context.config);
    if (!validation_errors.empty())
    {
        log_validation_errors(validation_errors);
        return nullptr;
    }

    if (!ModbusMasterAssembly::install(context))
    {
        Logger::error("[ApplicationBootstrap] failed to assemble Modbus Master stack");
        return nullptr;
    }

    HttpAssembly::install(context, std::move(http_route_provider_factory));

    return std::make_unique<ApplicationHost>(
        std::move(context.config),
        std::move(context.modbus_master_adapter),
        std::move(context.http_module_factory));
}
