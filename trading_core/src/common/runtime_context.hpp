/**============================================================================
Name        : runtime_context.hpp
Created on  : 29.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Shared runtime services context.
============================================================================**/

#ifndef TRADINGCOREBASE_RUNTIME_CONTEXT_HPP
#define TRADINGCOREBASE_RUNTIME_CONTEXT_HPP

#include "logger.hpp"
#include "metrics_collector.hpp"

namespace trading::common
{
    struct RuntimeContext
    {
        logging::ILogger& logger;
        metrics::MetricsCollector& metricsCollector;
    };
}

#endif //TRADINGCOREBASE_RUNTIME_CONTEXT_HPP