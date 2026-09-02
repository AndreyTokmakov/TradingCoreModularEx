/**============================================================================
Name        : have_metrics.hpp
Created on  : 01.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : have_metrics.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_HAVE_METRICS_HPP
#define TRADINGCOREMODULAREX_HAVE_METRICS_HPP

#include "metrics.hpp"

namespace trading::metrics
{
    struct HaveMetrics
    {
        void setMetrics(Metrics& metricsIn){
            this->metrics = &metricsIn;
        }

    protected:

        Metrics* metrics { nullptr };
    };
}

#endif //TRADINGCOREMODULAREX_HAVE_METRICS_HPP
