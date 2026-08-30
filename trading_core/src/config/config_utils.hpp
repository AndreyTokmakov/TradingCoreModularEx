/**============================================================================
Name        : config_utils.hpp
Created on  : 30.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : config_utils.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_CONFIG_UTILS_HPP
#define TRADINGCOREMODULAREX_CONFIG_UTILS_HPP

#include "config.hpp"
#include <string_view>

namespace trading::config
{
    [[nodiscard]]
    const ExchangeConfig& findExchange(const Config& config,
                                       std::string_view name);

    [[nodiscard]]
    Config loadConfig(const std::filesystem::path& configPath);
}

#endif //TRADINGCOREMODULAREX_CONFIG_UTILS_HPP
