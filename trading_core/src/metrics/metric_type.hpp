/**============================================================================
Name        : metric_type.hpp
Created on  : 29.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Defines metric types collected by the trading system.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_METRIC_TYPE_HPP
#define FINANCETECHNOLOGYPROJECTS_METRIC_TYPE_HPP

#include <cstdint>

namespace trading::metrics
{
    enum class MetricType : uint16_t
    {
        OrderRequests,
        ExecutionReport,
        MarketDataReceived,
        _Count
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_METRIC_TYPE_HPP