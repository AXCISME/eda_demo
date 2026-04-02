/**
 * HttpClientAssembly.h
 * 负责装配 HTTP client 协议栈
 */
#pragma once

#include "bootstrap/assembly/AssemblyContext.h"

class HttpClientAssembly
{
public:
    static void install(AssemblyContext& context);
};
