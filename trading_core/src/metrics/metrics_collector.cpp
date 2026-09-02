/**============================================================================
Name        : metrics_collector.hpp
Created on  : 29.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Registers per-thread metrics and aggregates them in the slow path.
============================================================================**/

#include "metrics_collector.hpp"

#include <ranges>


namespace trading::metrics
{
    Metrics& MetricsCollector::getThreadLocalMetrics() noexcept
    {
        std::lock_guard lock { mutex };
        const CpuId cpuId = ::sched_getcpu();
        return allMetrics.try_emplace(cpuId).first->second;
    }

    MetricsCollector& MetricsCollector::getCollector() noexcept
    {
        static MetricsCollector metrics_collector;
        return metrics_collector;
    }

    void MetricsCollector::aggregate() const
    {
        Metrics stats;
        {
            std::lock_guard<std::mutex> lock{mutex};
            for (const Metrics &metric: allMetrics | std::views::values)
            {
                stats += metric;
            }
        }

        // Future:
        // serialize stats
        // send stats
        // persist stats
        // publish stats
    }
}

