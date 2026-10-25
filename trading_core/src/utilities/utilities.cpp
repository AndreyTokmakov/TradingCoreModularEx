/**============================================================================
Name        : utilities.cpp
Created on  : 25.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : utilities.cpp
============================================================================**/

#include "utilities.hpp"

namespace
{
    using namespace std::string_view_literals;

    constexpr std::string_view HexDigits = "0123456789ABCDEF"sv;
    constexpr int64_t DecimalScale = 100'000'000;
}

namespace trading::utilities
{
    std::string urlEncode(const std::string_view value)
    {
        std::string result;
        result.reserve(value.size());
        for (const unsigned char character : value)
        {
            if (std::isalnum(character) || character == '-' || character == '_' || character == '.' || character == '~') {
                result.push_back(static_cast<char>(character));
            } else {
                result.push_back('%');
                result.push_back(HexDigits[character >> 4]);
                result.push_back(HexDigits[character & 0x0F]);
            }
        }
        return result;
    }


    int64_t parseDecimal(const std::string_view value)
    {
        bool negative = false;
        std::size_t index = 0;

        if (!value.empty() && value.front() == '-'){
            negative = true;
            index = 1;
        }

        int64_t integerPart = 0;
        while (index < value.size() && value[index] != '.'){
            integerPart = integerPart * 10 +static_cast<int64_t>(value[index] - '0');
            ++index;
        }

        int64_t fractionalPart = 0;
        std::size_t fractionalDigits = 0;

        if (index < value.size() && value[index] == '.')
        {
            ++index;
            while (index < value.size() && fractionalDigits < 8)
            {
                fractionalPart = fractionalPart * 10 + static_cast<int64_t>(value[index] - '0');
                ++fractionalDigits;
                ++index;
            }
        }

        while (fractionalDigits < 8) {
            fractionalPart *= 10;
            ++fractionalDigits;
        }

        const int64_t result = integerPart * DecimalScale + fractionalPart;
        return negative ? -result : result;
    }

    Price parsePrice(const std::string_view value) {
        return Price { parseDecimal(value) };
    }

    Quantity parseQuantity(const std::string_view value) {
        return Quantity { parseDecimal(value) };
    }
}