#include "bootstrap/AppConfigValidation.h"

#include "bootstrap/BuildFeatures.h"
#include "runtime/logging/Logger.h"

std::vector<std::string> validate_app_config(const AppConfig& config)
{
    std::vector<std::string> errors;

    if (config.http.enabled)
    {
        if (!build_features::http)
        {
            errors.push_back("HTTP is enabled in config but was not compiled into this build");
        }
        else
        {
            switch (config.http.backend)
            {
                case HttpBackendId::FAKE:
                    if (!build_features::http_fake)
                    {
                        errors.push_back("HTTP fake backend selected but not compiled into this build");
                    }
                    break;

                case HttpBackendId::MONGOOSE:
                    if (!build_features::http_mongoose)
                    {
                        errors.push_back("HTTP mongoose backend selected but not compiled into this build");
                    }
                    break;

                default:
                    errors.push_back("HTTP is enabled but backend is NONE");
                    break;
            }
        }

        if (config.http.port <= 0)
        {
            errors.push_back("HTTP port must be greater than 0");
        }
    }

    if (config.modbus_master.enabled)
    {
        if (!build_features::modbus_master)
        {
            errors.push_back("Modbus Master is enabled in config but was not compiled into this build");
        }
        else
        {
            switch (config.modbus_master.backend)
            {
                case ModbusMasterBackendId::FAKE:
                    if (!build_features::modbus_master_fake)
                    {
                        errors.push_back("Modbus Master fake backend selected but not compiled into this build");
                    }
                    break;

                case ModbusMasterBackendId::LIBMODBUS_MASTER_TCP:
                    if (!build_features::modbus_master_libmodbus_tcp)
                    {
                        errors.push_back("Modbus Master TCP libmodbus backend selected but not compiled into this build");
                    }
                    if (config.modbus_master.tcp_host.empty())
                    {
                        errors.push_back("Modbus Master TCP host must not be empty");
                    }
                    if (config.modbus_master.tcp_port <= 0)
                    {
                        errors.push_back("Modbus Master TCP port must be greater than 0");
                    }
                    break;

                case ModbusMasterBackendId::LIBMODBUS_MASTER_RTU:
                    if (!build_features::modbus_master_libmodbus_rtu)
                    {
                        errors.push_back("Modbus Master RTU libmodbus backend selected but not compiled into this build");
                    }
                    if (config.modbus_master.serial_device.empty())
                    {
                        errors.push_back("Modbus Master RTU serial_device must not be empty");
                    }
                    if (config.modbus_master.baudrate <= 0)
                    {
                        errors.push_back("Modbus Master RTU baudrate must be greater than 0");
                    }
                    if (config.modbus_master.parity != 'N'
                        && config.modbus_master.parity != 'E'
                        && config.modbus_master.parity != 'O')
                    {
                        errors.push_back("Modbus Master RTU parity must be one of N/E/O");
                    }
                    if (config.modbus_master.data_bits <= 0)
                    {
                        errors.push_back("Modbus Master RTU data_bits must be greater than 0");
                    }
                    if (config.modbus_master.stop_bits <= 0)
                    {
                        errors.push_back("Modbus Master RTU stop_bits must be greater than 0");
                    }
                    break;

                default:
                    errors.push_back("Modbus Master is enabled but backend is NONE");
                    break;
            }
        }

        if (config.modbus_master.poll_interval_ms <= 0)
        {
            errors.push_back("Modbus Master poll_interval_ms must be greater than 0");
        }
    }

    return errors;
}
