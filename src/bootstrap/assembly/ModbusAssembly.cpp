#include "bootstrap/assembly/ModbusAssembly.h"

#include "bootstrap/factory/ModbusBackendFactory.h"

bool ModbusAssembly::install(AssemblyContext& context)
{
    if (!context.config.modbus.enabled)
    {
        context.modbus_adapter.reset();
        return true;
    }

    context.modbus_adapter = ModbusBackendFactory::create(context.config.modbus);
    return context.modbus_adapter != nullptr;
}
