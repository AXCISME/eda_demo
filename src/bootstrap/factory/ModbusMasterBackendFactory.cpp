#include "bootstrap/factory/ModbusMasterBackendFactory.h"

#include "bootstrap/BuildFeatures.h"
#include "infrastructure/transport/modbus/FakeModbusMasterAdapter.h"
#include "runtime/logging/Logger.h"

#if EDA_ENABLE_MODBUS_MASTER && EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_TCP
#include "infrastructure/transport/modbus/LibmodbusMasterTcpAdapter.h"
#endif

#if EDA_ENABLE_MODBUS_MASTER && EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_RTU
#include "infrastructure/transport/modbus/LibmodbusMasterRtuAdapter.h"
#endif

bool ModbusMasterBackendFactory::is_compiled(ModbusMasterBackendId backend)
{
    switch (backend)
    {
        case ModbusMasterBackendId::FAKE:
            return build_features::modbus_master && build_features::modbus_master_fake;

        case ModbusMasterBackendId::LIBMODBUS_MASTER_TCP:
            return build_features::modbus_master && build_features::modbus_master_libmodbus_tcp;

        case ModbusMasterBackendId::LIBMODBUS_MASTER_RTU:
            return build_features::modbus_master && build_features::modbus_master_libmodbus_rtu;

        default:
            return false;
    }
}

std::unique_ptr<IModbusMasterAdapter> ModbusMasterBackendFactory::create(const ModbusMasterConfig& config)
{
    if (!config.enabled)
    {
        return nullptr;
    }

    switch (config.backend)
    {
        case ModbusMasterBackendId::FAKE:
            if (is_compiled(config.backend))
            {
                return std::make_unique<FakeModbusMasterAdapter>();
            }
            break;

        case ModbusMasterBackendId::LIBMODBUS_MASTER_TCP:
#if EDA_ENABLE_MODBUS_MASTER && EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_TCP
            if (is_compiled(config.backend))
            {
                return std::make_unique<LibmodbusMasterTcpAdapter>(
                    config.tcp_host,
                    config.tcp_port);
            }
            break;
#else
            break;
#endif

        case ModbusMasterBackendId::LIBMODBUS_MASTER_RTU:
#if EDA_ENABLE_MODBUS_MASTER && EDA_MODBUS_MASTER_BACKEND_LIBMODBUS_RTU
            if (is_compiled(config.backend))
            {
                return std::make_unique<LibmodbusMasterRtuAdapter>(
                    config.serial_device,
                    config.baudrate,
                    config.parity,
                    config.data_bits,
                    config.stop_bits);
            }
            break;
#else
            break;
#endif

        default:
            break;
    }

    Logger::error(
        std::string("[ModbusMasterBackendFactory] backend unavailable: ")
        + to_string(config.backend));
    return nullptr;
}
