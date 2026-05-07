/**
 * ModbusMasterBackendFactory.h
 * 负责按配置和编译能力创建具体的 Modbus Master backend
 */
#pragma once

#include <memory>

#include "bootstrap/AppConfig.h"

class IModbusMasterAdapter;

class ModbusMasterBackendFactory
{
public:
    static bool is_compiled(ModbusMasterBackendId backend);
    static std::unique_ptr<IModbusMasterAdapter> create(const ModbusDeviceConfig& config);
};
