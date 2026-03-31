#include "bootstrap/factory/HttpBackendFactory.h"

#include "bootstrap/BuildFeatures.h"
#include "infrastructure/transport/http/FakeHttpServerAdapter.h"
#include "runtime/logging/Logger.h"

#if EDA_ENABLE_HTTP && EDA_HTTP_BACKEND_MONGOOSE
#include "infrastructure/transport/http/MongooseHttpServerAdapter.h"
#endif

bool HttpBackendFactory::is_compiled(HttpBackendId backend)
{
    switch (backend)
    {
        case HttpBackendId::FAKE:
            return build_features::http && build_features::http_fake;

        case HttpBackendId::MONGOOSE:
            return build_features::http && build_features::http_mongoose;

        default:
            return false;
    }
}

std::unique_ptr<IHttpServerAdapter> HttpBackendFactory::create(const HttpConfig& config)
{
    if (!config.enabled)
    {
        return nullptr;
    }

    switch (config.backend)
    {
        case HttpBackendId::FAKE:
            if (is_compiled(config.backend))
            {
                return std::make_unique<FakeHttpServerAdapter>();
            }
            break;

        case HttpBackendId::MONGOOSE:
#if EDA_ENABLE_HTTP && EDA_HTTP_BACKEND_MONGOOSE
            if (is_compiled(config.backend))
            {
                return std::make_unique<MongooseHttpServerAdapter>();
            }
            break;
#else
            break;
#endif

        default:
            break;
    }

    Logger::error(
        std::string("[HttpBackendFactory] backend unavailable: ")
        + to_string(config.backend));
    return nullptr;
}
