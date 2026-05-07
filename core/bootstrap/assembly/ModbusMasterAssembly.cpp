#include "bootstrap/assembly/ModbusMasterAssembly.h"

#include "bootstrap/factory/ModbusMasterBackendFactory.h"

bool ModbusMasterAssembly::install(AssemblyContext& context)
{
    if (!context.config.modbus_master.enabled)
    {
        context.modbus_master_adapters.clear();
        return true;
    }

    context.modbus_master_adapters.clear();
    for (const auto& device : context.config.modbus_master.devices)
    {
        auto adapter = ModbusMasterBackendFactory::create(device);
        if (!adapter)
        {
            return false;
        }

        context.modbus_master_adapters.push_back(std::move(adapter));
    }

    return true;
}
