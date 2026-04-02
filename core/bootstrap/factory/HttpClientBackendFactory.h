/**
 * HttpClientBackendFactory.h
 * 负责按配置和编译能力创建具体的 HTTP client backend
 */
#pragma once

#include <memory>

#include "bootstrap/AppConfig.h"

class IHttpClientAdapter;

class HttpClientBackendFactory
{
public:
    static bool is_compiled(HttpClientBackendId backend);
    static std::unique_ptr<IHttpClientAdapter> create(const HttpClientConfig& config);
};
