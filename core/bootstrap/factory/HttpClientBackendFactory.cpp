#include "bootstrap/factory/HttpClientBackendFactory.h"

#include "bootstrap/BuildFeatures.h"
#include "infrastructure/transport/http/FakeHttpClientAdapter.h"
#include "runtime/logging/Logger.h"

#if EDA_ENABLE_HTTP_CLIENT && EDA_HTTP_CLIENT_BACKEND_MONGOOSE
#include "infrastructure/transport/http/MongooseHttpClientAdapter.h"
#endif

bool HttpClientBackendFactory::is_compiled(HttpClientBackendId backend)
{
    switch (backend)
    {
        case HttpClientBackendId::FAKE:
            return build_features::http_client && build_features::http_client_fake;

        case HttpClientBackendId::MONGOOSE:
            return build_features::http_client && build_features::http_client_mongoose;

        default:
            return false;
    }
}

std::unique_ptr<IHttpClientAdapter> HttpClientBackendFactory::create(const HttpClientConfig& config)
{
    if (!config.enabled)
    {
        return nullptr;
    }

    switch (config.backend)
    {
        case HttpClientBackendId::FAKE:
            if (is_compiled(config.backend))
            {
                return std::make_unique<FakeHttpClientAdapter>();
            }
            break;

        case HttpClientBackendId::MONGOOSE:
#if EDA_ENABLE_HTTP_CLIENT && EDA_HTTP_CLIENT_BACKEND_MONGOOSE
            if (is_compiled(config.backend))
            {
                return std::make_unique<MongooseHttpClientAdapter>();
            }
            break;
#else
            break;
#endif

        default:
            break;
    }

    Logger::error(
        std::string("[HttpClientBackendFactory] backend unavailable: ")
        + to_string(config.backend));
    return nullptr;
}
