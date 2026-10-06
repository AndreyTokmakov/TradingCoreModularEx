/**============================================================================
Name        : binance_execution_report_source.cpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance execution report source implementation.
============================================================================**/

#include "binance_execution_report_source.hpp"

namespace trading::exchanges::binance
{
    BinanceExecutionReportSource::BinanceExecutionReportSource(std::string endpoint) noexcept :
        endpoint { std::move(endpoint) }
    {
    }

    void BinanceExecutionReportSource::setReportHandler(execution::IExecutionReportHandler& handler) {
        reportHandler = &handler;
    }

    void BinanceExecutionReportSource::start()
    {
        if (running)
            return;
        running = true;

        /*
          Establish Binance execution WebSocket connection here.
          Incoming Binance messages must be parsed and converted into trading::execution::ExecutionReport.
          After successful normalization:
              reportHandler->onExecutionReport(report);
        */

        std::string_view execReportData = "report";
        while (false /** running **/) {
            reportHandler->onExecutionReport(execReportData);
        }
    }

    void BinanceExecutionReportSource::stop()
    {
        running = false;
    }
}