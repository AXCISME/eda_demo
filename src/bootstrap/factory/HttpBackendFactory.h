/**
 * HttpBackendFactory.h
 * 负责按配置和编译能力创建具体的HTTP backend
 */
#pragma once

#include <memory>

#include "bootstrap/AppConfig.h"

class IHttpServerAdapter;

class HttpBackendFactory
{
public:
    static bool is_compiled(HttpBackendId backend);
    static std::unique_ptr<IHttpServerAdapter> create(const HttpConfig& config);
};
