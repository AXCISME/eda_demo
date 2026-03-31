/**
 * ModbusBackendFactory.h
 * 负责按配置和编译能力创建具体的Modbus backend
 */
#pragma once

#include <memory>

#include "bootstrap/AppConfig.h"

class IModbusMasterAdapter;

class ModbusBackendFactory
{
public:
    static bool is_compiled(ModbusBackendId backend);
    static std::unique_ptr<IModbusMasterAdapter> create(const ModbusConfig& config);
};
