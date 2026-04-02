#include "bootstrap/assembly/ModbusMasterAssembly.h"

#include "bootstrap/factory/ModbusMasterBackendFactory.h"

bool ModbusMasterAssembly::install(AssemblyContext& context)
{
    if (!context.config.modbus_master.enabled)
    {
        context.modbus_master_adapter.reset();
        return true;
    }

    context.modbus_master_adapter = ModbusMasterBackendFactory::create(context.config.modbus_master);
    return context.modbus_master_adapter != nullptr;
}
