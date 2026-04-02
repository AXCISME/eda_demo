/**
 * IHttpRouteProvider.h
 * 定义"谁来注册路由"，需要框架外部继承
 * C++ 抽象类，必须被继承实现
 */
#pragma once
#include "interfaces/http/IHttpRouteRegistry.h"

class IHttpRouteProvider
{
public:
    virtual ~IHttpRouteProvider() = default;

    // 虚函数，继承者必须实现路由注册，调用者必须提供IHttpRouteRegistry的实现
    virtual void register_routes(IHttpRouteRegistry& registry) = 0;
};
