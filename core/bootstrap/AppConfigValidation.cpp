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

    if (config.http_client.enabled)
    {
        if (!build_features::http_client)
        {
            errors.push_back("HTTP client is enabled in config but was not compiled into this build");
        }
        else
        {
            switch (config.http_client.backend)
            {
                case HttpClientBackendId::FAKE:
                    if (!build_features::http_client_fake)
                    {
                        errors.push_back("HTTP client fake backend selected but not compiled into this build");
                    }
                    break;

                case HttpClientBackendId::MONGOOSE:
                    if (!build_features::http_client_mongoose)
                    {
                        errors.push_back("HTTP client mongoose backend selected but not compiled into this build");
                    }
                    break;

                default:
                    errors.push_back("HTTP client is enabled but backend is NONE");
                    break;
            }
        }
    }

    if (config.modbus_master.enabled)
    {
        if (!build_features::modbus_master)
        {
            errors.push_back("Modbus Master is enabled in config but was not compiled into this build");
        }

        if (config.modbus_master.devices.empty())
        {
            errors.push_back("Modbus Master is enabled but devices is empty");
        }

        for (const auto& device : config.modbus_master.devices)
        {
            const std::string prefix = "Modbus device '" + device.device_name + "': ";

            if (device.device_name.empty())
            {
                errors.push_back("Modbus device_name must not be empty");
            }

            switch (device.backend)
            {
                case ModbusMasterBackendId::FAKE:
                    if (!build_features::modbus_master_fake)
                    {
                        errors.push_back(prefix + "fake backend selected but not compiled into this build");
                    }
                    break;

                case ModbusMasterBackendId::LIBMODBUS_MASTER_TCP:
                    if (!build_features::modbus_master_libmodbus_tcp)
                    {
                        errors.push_back(prefix + "TCP libmodbus backend selected but not compiled into this build");
                    }
                    if (device.tcp_host.empty())
                    {
                        errors.push_back(prefix + "TCP host must not be empty");
                    }
                    if (device.tcp_port <= 0)
                    {
                        errors.push_back(prefix + "TCP port must be greater than 0");
                    }
                    break;

                case ModbusMasterBackendId::LIBMODBUS_MASTER_RTU:
                    if (!build_features::modbus_master_libmodbus_rtu)
                    {
                        errors.push_back(prefix + "RTU libmodbus backend selected but not compiled into this build");
                    }
                    if (device.serial_device.empty())
                    {
                        errors.push_back(prefix + "RTU serial_device must not be empty");
                    }
                    if (device.baudrate <= 0)
                    {
                        errors.push_back(prefix + "RTU baudrate must be greater than 0");
                    }
                    if (device.parity != 'N'
                        && device.parity != 'E'
                        && device.parity != 'O')
                    {
                        errors.push_back(prefix + "RTU parity must be one of N/E/O");
                    }
                    if (device.data_bits <= 0)
                    {
                        errors.push_back(prefix + "RTU data_bits must be greater than 0");
                    }
                    if (device.stop_bits <= 0)
                    {
                        errors.push_back(prefix + "RTU stop_bits must be greater than 0");
                    }
                    break;

                default:
                    errors.push_back(prefix + "backend is NONE");
                    break;
            }

            if (device.retry_delay_ms <= 0)
            {
                errors.push_back(prefix + "retry_delay_ms must be greater than 0");
            }
        }
    }

    return errors;
}
