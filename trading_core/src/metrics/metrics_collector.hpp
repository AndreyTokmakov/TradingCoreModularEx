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

#include <map>
#include <mutex>
#include <thread>

namespace trading::metrics
{
    class MetricsCollector
    {
        using CpuId = int32_t;

    public:
        MetricsCollector(const MetricsCollector&) = delete;
        MetricsCollector& operator=(const MetricsCollector&) = delete;

        MetricsCollector(MetricsCollector&&) = delete;
        MetricsCollector& operator=(MetricsCollector&&) = delete;


        // TODO: Add description -> result shall be thread_local
        [[nodiscard]]
        Metrics& getThreadLocalMetrics() noexcept;

        [[nodiscard]]
        static MetricsCollector& getCollector() noexcept;

        [[maybe_unused]]
        void aggregate() const;

    private:

        MetricsCollector() = default;

        mutable std::mutex mutex;
        std::map<CpuId, Metrics> allMetrics {};
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_METRICS_COLLECTOR_HPP