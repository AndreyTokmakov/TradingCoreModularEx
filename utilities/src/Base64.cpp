/**============================================================================
Name        : Base64.cpp
Created on  : 24.03.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Base64.h
============================================================================**/

#include "Base64.hpp"
#include <iostream>
#include <array>
#include <cstdint>

namespace
{
    constexpr std::array<char, 64> ENCODING_TABLE
    {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
            'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
            'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
            'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
            'w', 'x', 'y', 'z', '0', '1', '2', '3',
            '4', '5', '6', '7', '8', '9', '+', '/'
    };

    // constexpr char PADDING { '=' };
    constexpr size_t MAX_BITS_TO_ENCODE {6};
    constexpr size_t BITS_IN_BYTE {8};
    constexpr size_t BYTES_PER_BLOCK { 3 };
    constexpr size_t OUTPUT_CHARS_PER_BLOCK { 4 };
}

namespace utilities::bas64
{
    std::string base64Encode(const std::string &str)
    {
        const size_t outputSize {
            OUTPUT_CHARS_PER_BLOCK * ((str.size() + BYTES_PER_BLOCK - 1) / BYTES_PER_BLOCK)
        };

        std::string result;
        result.reserve(outputSize);

        for (size_t index { 0 }; index < str.size(); index += 3)
        {
            const uint8_t byte0 { static_cast<uint8_t>(str[index]) };
            const uint8_t byte1 { index + 1 < str.size() ? static_cast<uint8_t>(str[index + 1]) : static_cast<uint8_t>(0) };
            const uint8_t byte2 { index + 2 < str.size() ? static_cast<uint8_t>(str[index + 2]) : static_cast<uint8_t>(0) };

            result.push_back(ENCODING_TABLE[byte0 >> 2]);
            result.push_back(ENCODING_TABLE[((byte0 & 0x03) << 4) | (byte1 >> 4)]);

            if (index + 1 < str.size())
                result.push_back(ENCODING_TABLE[((byte1 & 0x0f) << 2) | (byte2 >> 6)]);
            else
                result.push_back('=');

            if (index + 2 < str.size())
                result.push_back(ENCODING_TABLE[byte2 & 0x3f]);
            else
                result.push_back('=');
        }

        return result;
    }
};
