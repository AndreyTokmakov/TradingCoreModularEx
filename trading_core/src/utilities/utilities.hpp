/**============================================================================
Name        : tilities.hpp
Created on  : 25.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : tilities.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_UTILITIES_HPP
#define TRADINGCOREMODULAREX_UTILITIES_HPP

#include <string_view>
#include <core/price.hpp>
#include <core/quantity.hpp>

namespace trading::utilities
{
    [[nodiscard]]
    std::string urlEncode(std::string_view value);

    [[nodiscard]]
    int64_t parseDecimal(std::string_view value);

    [[nodiscard]]
    Price parsePrice(std::string_view value);

    [[nodiscard]]
    Quantity parseQuantity(std::string_view value);
}

#endif //TRADINGCOREMODULAREX_UTILITIES_HPP
