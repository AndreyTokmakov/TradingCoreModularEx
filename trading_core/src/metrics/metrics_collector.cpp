/**============================================================================
Name        : metrics_collector.hpp
Created on  : 29.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Registers per-thread metrics and aggregates them in the slow path.
============================================================================**/

#include "metrics_collector.hpp"

namespace trading::metrics
{
    Metrics& MetricsCollector::getThreadMetrics() noexcept
    {
        std::lock_guard<std::mutex> lock{mutex};
        thread_local Metrics& metrics = allMetrics.emplace_back();
        return metrics;
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
            for (const auto& metric : allMetrics)
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

