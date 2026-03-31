/**
 * AssmblyContext.h
 * 装配过程中的"中转盒子"
 * 各个assembly的中间结果
 */
#pragma once

#include <memory>
#include <utility>

#include "application/host/ApplicationHost.h"
#include "bootstrap/AppConfig.h"
#include "infrastructure/transport/modbus/IModbusMasterAdapter.h"

struct AssemblyContext
{
    explicit AssemblyContext(AppConfig config_in)
        : config(std::move(config_in))
    {
    }

    AppConfig config;
    std::unique_ptr<IModbusMasterAdapter> modbus_master_adapter;
    ApplicationHost::HttpModuleFactory http_module_factory;
};
