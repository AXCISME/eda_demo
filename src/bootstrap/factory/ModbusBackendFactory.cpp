#include "bootstrap/factory/ModbusBackendFactory.h"

#include "bootstrap/BuildFeatures.h"
#include "infrastructure/transport/modbus/FakeModbusMasterAdapter.h"
#include "runtime/logging/Logger.h"

bool ModbusBackendFactory::is_compiled(ModbusBackendId backend)
{
    switch (backend)
    {
        case ModbusBackendId::FAKE:
            return build_features::modbus && build_features::modbus_fake;

        default:
            return false;
    }
}

std::unique_ptr<IModbusMasterAdapter> ModbusBackendFactory::create(const ModbusConfig& config)
{
    if (!config.enabled)
    {
        return nullptr;
    }

    switch (config.backend)
    {
        case ModbusBackendId::FAKE:
            if (is_compiled(config.backend))
            {
                return std::make_unique<FakeModbusMasterAdapter>();
            }
            break;

        default:
            break;
    }

    Logger::error(
        std::string("[ModbusBackendFactory] backend unavailable: ")
        + to_string(config.backend));
    return nullptr;
}
