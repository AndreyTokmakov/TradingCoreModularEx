/**============================================================================
Name        : hugh_resolution_timer.hpp
Created on  : 28.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : hugh_resolution_timer.hpp
============================================================================**/

#ifndef TRADINGCOREBASE_HUGH_RESOLUTION_TIMER_HPP
#define TRADINGCOREBASE_HUGH_RESOLUTION_TIMER_HPP

#include <chrono>
#include <thread>
#include <x86intrin.h>

namespace trading::metrics
{
    // Используем TSC (Time Stamp Counter) для максимальной точности
    class HighResolutionTimer
    {
        using value_type = uint64_t;
        static inline value_type tsc_frequency_ = 0;

    public:
        static value_type get_tsc_frequency()
        {
            if (tsc_frequency_ == 0)
            {
                // Калибровка через std::chrono
                const auto start = std::chrono::steady_clock::now();
                const value_type tsc_start = __rdtsc();

                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                const auto end = std::chrono::steady_clock::now();
                const value_type tsc_end = __rdtsc();

                const auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                tsc_frequency_ = (tsc_end - tsc_start) * 1'000'000'000ULL / duration_ns;
            }
            return tsc_frequency_;
        }

        static value_type now() noexcept {
            return __rdtsc();
        }

        static value_type elapsed_ns(const value_type start, const value_type end) noexcept {
            return (end - start) * 1'000'000'000ULL / get_tsc_frequency();
        }
    };
}

#endif //TRADINGCOREBASE_HUGH_RESOLUTION_TIMER_HPP
