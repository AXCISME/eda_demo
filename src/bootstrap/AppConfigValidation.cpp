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

    if (config.modbus.enabled)
    {
        if (!build_features::modbus)
        {
            errors.push_back("Modbus is enabled in config but was not compiled into this build");
        }
        else
        {
            switch (config.modbus.backend)
            {
                case ModbusBackendId::FAKE:
                    if (!build_features::modbus_fake)
                    {
                        errors.push_back("Modbus fake backend selected but not compiled into this build");
                    }
                    break;

                default:
                    errors.push_back("Modbus is enabled but backend is NONE");
                    break;
            }
        }

        if (config.modbus.poll_interval_ms <= 0)
        {
            errors.push_back("Modbus poll_interval_ms must be greater than 0");
        }
    }

    return errors;
}
