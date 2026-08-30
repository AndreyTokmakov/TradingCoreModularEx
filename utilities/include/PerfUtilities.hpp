/**============================================================================
Name        : PerfUtilities.h
Created on  : 21.11.2023
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Utilities.h
============================================================================**/

#ifndef CPPPROJECTS_UTILITIES_HPP
#define CPPPROJECTS_UTILITIES_HPP

#include <string_view>
#include <chrono>
#include <x86intrin.h>

namespace utilities::perf
{
    struct NonCopyableMovable
    {
        NonCopyableMovable() = default;

        NonCopyableMovable(const NonCopyableMovable&) = delete;
        NonCopyableMovable(NonCopyableMovable&&) = delete;
        NonCopyableMovable& operator=(const NonCopyableMovable&) = delete;
        NonCopyableMovable& operator=(NonCopyableMovable&&) = delete;
    };

    struct TimerBase: NonCopyableMovable
    {
        const std::string_view benchmarkName;

        explicit TimerBase(const std::string_view info) :
                benchmarkName { info } {
        }
    };

    struct ScopedTimer: TimerBase
    {
        using ClockType = std::chrono::high_resolution_clock;
        using TimePoint = ClockType::time_point;
        using Duration  = double;

        explicit ScopedTimer(const std::string_view info,
                             const bool warmUp = false) : TimerBase { info }, warmUp { warmUp } {
        }

        Duration getElapsedTime() const noexcept;
        Duration getElapsedTimeAndReset() noexcept;

        ~ScopedTimer();

    private:

        [[nodiscard]]
        static Duration getDuration(TimePoint from,
                                    TimePoint to = ClockType::now()) noexcept;
    private:

        TimePoint start { ClockType::now() };
        const bool warmUp { false };
    };

    struct TSCScopedTimer: TimerBase
    {
        const uint64_t start = __rdtsc();

        explicit TSCScopedTimer(const std::string_view info) : TimerBase(info) {
        }

        ~TSCScopedTimer();
    };
}

#endif //CPPPROJECTS_UTILITIES_HPP
