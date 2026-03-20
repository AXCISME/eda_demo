#include "app/Application.h"
#include <memory>

#include "modules/http/BaseHttpRouteProvider.h"
#include "transport/modbus/FakeModbusMasterAdapter.h"
#include "transport/http/FakeHttpServerAdapter.h"
#if EDA_ENABLE_HTTP
#include "transport/http/MongooseHttpServerAdapter.h"
#endif

namespace {
std::unique_ptr<IHttpServerAdapter> create_http_adapter(const AppConfig& config)
{
    if (config.http_backend == "fake")
    {
        return std::make_unique<FakeHttpServerAdapter>();
    }

    if (config.http_backend == "mongoose")
    {
#if EDA_ENABLE_HTTP
        return std::make_unique<MongooseHttpServerAdapter>();
#else
        Logger::warn("[Application] mongoose backend unavailable because EDA_ENABLE_HTTP=0");
        return nullptr;
#endif
    }

    Logger::error("[Application] unknown http backend: " + config.http_backend);
    return nullptr;
}
}

Application::Application(
    AppConfig config,
    HttpRouteProviderFactory http_route_provider_factory)
    :   config_(std::move(config)),
        modbus_(bus_, std::make_unique<FakeModbusMasterAdapter>()),
        ws_(bus_),
        control_(bus_),
        loop_(bus_)
{
    if (config_.enable_http)
    {
        auto adapter = create_http_adapter(config_);
        if (adapter)
        {
            auto route_provider = http_route_provider_factory
                ? http_route_provider_factory(bus_)
                : std::make_unique<BaseHttpRouteProvider>();

            http_ = std::make_unique<HttpModule>(
                std::move(adapter),
                std::move(route_provider));
        }
        
    }
    
}

void Application::init()
{
    modbus_.init();
    if (!modbus_.start())
    {
        Logger::error("[Application] failed to start Modbus master");
    }

    ws_.init();
    control_.init();

    if (http_)
    {
        http_->init();
        if (!http_->start(config_.http_host, config_.http_port))
        {
            Logger::error("[Application] failed to start HTTP server");
        }
        
    }
}

void Application::run()
{
    if (http_)
    {
        Logger::info("[Application] HTTP server ready on :" + std::to_string(config_.http_port));
        Logger::info("[Application] try:");
        Logger::info("[Application]   GET  /ping");
        Logger::info("[Application]   GET  /status");
        Logger::info("[Application]   POST /control  body: addr=100&value=1");
    }
    
    loop_.run();
}

void Application::stop()
{
    if (http_)
    {
        http_->stop();
    }

    modbus_.stop();
    loop_.stop();
}

void Application::simulate_ws_client_connect(const std::string& client_id)
{
    ws_.simulate_client_connect(client_id);
}

void Application::simulate_ws_client_message(const std::string& client_id, int addr, int value)
{
    ws_.simulate_client_message(client_id, addr, value);
}
