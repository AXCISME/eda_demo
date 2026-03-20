#pragma once
#include <string>
#include <unordered_map>

struct HttpResponse
{
    int status {200};
    std::string body;
    std::unordered_map<std::string, std::string> headers;
};
