/**============================================================================
Name        : metrics.hpp
Created on  : 29.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Collection of counters for a single execution thread.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_METRICS_HPP
#define FINANCETECHNOLOGYPROJECTS_METRICS_HPP

#include "counter.hpp"
#include "metric_type.hpp"

#include <array>
#include <cstddef>

#include "ThreadUtilities.hpp"         // NOTE: Remove
#include <iostream>                    // NOTE: Remove

namespace trading::metrics
{
    class Metrics
    {
    public:
        using counter_type = Counter::counter_type;

        constexpr Metrics() noexcept = default;

        Metrics(const Metrics&) = delete;
        Metrics& operator=(const Metrics&) = delete;

        template<MetricType type>
        void increment(const counter_type value = 1) noexcept
        {
            counters[toIndex(type)].increment(value);
            std::cout << utilities::threading::getCpu() << std::endl;     // NOTE: Remove
        }

        template<MetricType type>
        [[nodiscard]]
        counter_type value() const noexcept
        {
            return counters[toIndex(type)].value();
        }

        void increment(const MetricType type, const counter_type value = 1) noexcept
        {
            counters[toIndex(type)].increment(value);
        }

        [[nodiscard]]
        counter_type value(const MetricType type) const noexcept
        {
            return counters[toIndex(type)].value();
        }

        Metrics& operator+=(const Metrics& other) noexcept
        {
            for (std::size_t index = 0; index < counters.size(); ++index)
                counters[index].increment(other.counters[index].value());

            return *this;
        }

    private:

        [[nodiscard]]
        static constexpr std::size_t toIndex(const MetricType type) noexcept
        {
            return static_cast<std::size_t>(type);
        }

        std::array<Counter, static_cast<std::size_t>(MetricType::_Count)> counters {};
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_METRICS_HPP