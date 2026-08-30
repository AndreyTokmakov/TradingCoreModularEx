/**============================================================================
Name        : Random.hpp
Created on  : 02.12.2025
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Random.hpp
============================================================================**/

#ifndef CPPPROJECTS_RANDOM_HPP
#define CPPPROJECTS_RANDOM_HPP

#include <random>
#include <stdexcept>

namespace utilities::random
{
    static inline std::random_device rd{};
    static inline std::mt19937 generator = std::mt19937 {rd()};

    template<typename Ty>
    [[nodiscard]]
    Ty getRandomInRange(const Ty start, const Ty end) noexcept
    {
        if constexpr (std::is_integral_v<Ty>) {
            return std::uniform_int_distribution<Ty> {start, end}(generator);
        }
        else if constexpr (std::is_floating_point_v<Ty>) {
            return std::uniform_real_distribution<Ty> {start, end}(generator);
        }
        else {
            throw std::invalid_argument("getRandomInRange: Invalid type");
        }
    }

    [[nodiscard]]
    inline std::string randomString(size_t size = 32)
    {
        std::uniform_int_distribution<> distribution{static_cast<int>('a'), static_cast<int>('z')};
        std::string str;
        str.reserve(size);
        while (size-- > 0)
            str.push_back(static_cast<char>(distribution(generator)));
        return str;
    }

    [[nodiscard]]
    inline int32_t getRandomInt(const int32_t from = 0,
                                const int32_t until = 10000)
    {
        std::uniform_int_distribution<> distribution(from, until);
        return distribution(generator);
    }
}

#endif //CPPPROJECTS_RANDOM_HPP