/**============================================================================
Name        : PerfUtilities.h
Created on  : 21.11.2023
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Utilities.h
============================================================================**/

#include "PerfUtilities.hpp"
#include <iostream>
#include <iomanip>


namespace utilities::perf
{
    ScopedTimer::~ScopedTimer()
    {
        const Duration duration = getDuration( start);
        if (false == warmUp) {
            std::cout << std::left << std::setw(14) << benchmarkName << ":  "<< duration << " seconds.\n";
        }
    }

    ScopedTimer::Duration ScopedTimer::getElapsedTime() const noexcept {
        return getDuration( start);
    }

    ScopedTimer::Duration ScopedTimer::getElapsedTimeAndReset() noexcept
    {
        const TimePoint now = ClockType::now();
        const Duration duration = getDuration( start, now);
        start = now;
        return duration;
    }

    ScopedTimer::Duration ScopedTimer::getDuration(const TimePoint from, const TimePoint to) noexcept {
        return duration_cast<std::chrono::duration<Duration>>(to - from).count();
    }
}


namespace utilities::perf
{
    TSCScopedTimer::~TSCScopedTimer()
    {
        const uint64_t duration =  __rdtsc() - start;
        std::cout << std::left << std::setw(14) << benchmarkName << ":  " << duration << ".\n";
    }
}




