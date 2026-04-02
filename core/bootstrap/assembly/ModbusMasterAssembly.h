/**
 * ModbusMasterAssembly.h
 * 负责装配 Modbus Master 协议栈
 */
#pragma once

#include "bootstrap/assembly/AssemblyContext.h"

class ModbusMasterAssembly
{
public:
    static bool install(AssemblyContext& context);
};
