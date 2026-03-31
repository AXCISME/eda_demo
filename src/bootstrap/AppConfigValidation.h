/**
 * AppConfigValidation.h
 * 做配置是否合法化检查，运行时检查
 */
#pragma once

#include <string>
#include <vector>

#include "bootstrap/AppConfig.h"

std::vector<std::string> validate_app_config(const AppConfig& config);
