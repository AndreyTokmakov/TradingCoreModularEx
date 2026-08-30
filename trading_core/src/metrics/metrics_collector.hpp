/**============================================================================
Name        : metrics_collector.hpp
Created on  : 29.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Registers per-thread metrics and aggregates them in the slow path.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_METRICS_COLLECTOR_HPP
#define FINANCETECHNOLOGYPROJECTS_METRICS_COLLECTOR_HPP

#include "metrics.hpp"

#include <list>
#include <mutex>

namespace trading::metrics
{
    class MetricsCollector
    {
    public:
        MetricsCollector(const MetricsCollector&) = delete;
        MetricsCollector& operator=(const MetricsCollector&) = delete;

        MetricsCollector(MetricsCollector&&) = delete;
        MetricsCollector& operator=(MetricsCollector&&) = delete;

        [[nodiscard]]
        Metrics& getThreadMetrics() noexcept;

        [[nodiscard]]
        static MetricsCollector& getCollector() noexcept;

        [[maybe_unused]]
        void aggregate() const;

    private:

        MetricsCollector() = default;

        mutable std::mutex mutex;
        std::list<Metrics> allMetrics {};
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_METRICS_COLLECTOR_HPP