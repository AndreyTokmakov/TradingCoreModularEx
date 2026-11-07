/**============================================================================
Name        : utils.hpp
Created on  : 07.11.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance Utilities
============================================================================**/

#ifndef TRADINGCOREMODULAREX_BINANCE_UTILITIES_HPP
#define TRADINGCOREMODULAREX_BINANCE_UTILITIES_HPP

#include <optional>
#include <string_view>

namespace trading::utilities::binance
{
    [[nodiscard]]
    std::optional<std::string> createSignature(std::string_view payload,
                                               std::string_view secretKey) noexcept;
}

#endif //TRADINGCOREMODULAREX_BINANCE_UTILITIES_HPP
