#pragma once
#include <string>
#include <unordered_map>

struct HttpRequest
{
    std::string method;
    std::string path;
    std::string query_string;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
};

