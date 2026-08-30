 /**============================================================================
Name        : counter.hpp
Created on  : 15.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : counter.hpp
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_METRICS_COUNTER_HPP
#define FINANCETECHNOLOGYPROJECTS_METRICS_COUNTER_HPP

#include <atomic>

namespace trading::metrics
{
    class Counter
    {
    public:
        constexpr Counter() noexcept = default;
        using counter_type = uint64_t;

        Counter(const Counter&) = delete;
        Counter& operator=(const Counter&) = delete;

        void increment(const counter_type value = 1) noexcept
        {
            value_.fetch_add(value, std::memory_order_relaxed);
        }

        [[nodiscard]]
        counter_type value() const noexcept {
            return value_.load(std::memory_order_relaxed);
        }

        void reset() noexcept {
            value_.store(0, std::memory_order_relaxed);
        }

        counter_type getAndReset() noexcept {
            return value_.exchange(0, std::memory_order_relaxed);
        }

    private:
        std::atomic<counter_type> value_ { 0 };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_METRICS_COUNTER_HPP