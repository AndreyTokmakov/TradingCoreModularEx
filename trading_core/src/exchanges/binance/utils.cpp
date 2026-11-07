/**============================================================================
Name        : utils.cpp
Created on  : 07.11.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : utils.cpp
============================================================================**/

#include "utils.hpp"
#include "utilities/utilities.hpp"
#include <openssl/hmac.h>

namespace trading::utilities::binance
{
    std::optional<std::string> createSignature(const std::string_view payload,
                                               const std::string_view secretKey) noexcept
    {
        if (secretKey.size() > static_cast<std::size_t>( std::numeric_limits<int>::max())) {
            return std::nullopt;
        }

        std::array<unsigned char, EVP_MAX_MD_SIZE> digest {};
        unsigned int digestLength = 0;
        const unsigned char* result = HMAC(EVP_sha256(),
                                           secretKey.data(),
                                           static_cast<int>(secretKey.size()),
                                           reinterpret_cast<const unsigned char*>(payload.data()),
                                           payload.size(),
                                           digest.data(),
                                           &digestLength);
        if (nullptr == result) {
            return std::nullopt;
        }
        return toHex(std::span { digest.data(), digestLength });
    }
}
