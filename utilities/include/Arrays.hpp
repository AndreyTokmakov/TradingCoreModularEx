/**============================================================================
Name        : Arrays.hpp
Created on  : 24.03.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Base64.h
============================================================================**/

#ifndef CPPPROJECTS_ARRAYS_HPP
#define CPPPROJECTS_ARRAYS_HPP

#include <array>

namespace utilities::arrays
{
    template<typename T>
    constexpr auto array_cat() -> std::array<T, 0>
    {
        return {};
    }

    template<typename T, std::size_t... Ss>
    constexpr auto array_cat(const std::array<T, Ss>&... arrays) -> std::array<T, (Ss + ...)>
    {
        std::array<T, (Ss + ...)> result {};
        auto itr = result.begin();
        ((itr = std::copy(arrays.begin(), arrays.end(), itr)), ...);
        return result;
    }
}


#endif //CPPPROJECTS_ARRAYS_HPP
