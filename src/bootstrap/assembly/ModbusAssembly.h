/**
 * ModbusAssembly.h
 * 负责装配 Modbus 协议栈
 */
#pragma once

#include "bootstrap/assembly/AssemblyContext.h"

class ModbusAssembly
{
public:
    static bool install(AssemblyContext& context);
};
