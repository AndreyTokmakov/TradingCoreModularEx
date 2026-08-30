/**============================================================================
Name        : binance_execution_report_source.hpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance execution report source.
============================================================================**/

/*
    BinanceExecutionReportSource represents the inbound execution event
    boundary for Binance.

    The current implementation is intentionally transport-independent.

    It provides an emit() method that can be used by a future Binance
    transport implementation to deliver normalized ExecutionReport objects.

    Current flow:

        Test / Binance transport
                  |
                  | ExecutionReport
                  v
        BinanceExecutionReportSource
                  |
                  v
        ExecutionReportHandler

    A real Binance WebSocket implementation can later replace the emit()
    call without changing the trading core.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_BINANCE_EXECUTION_REPORT_SOURCE_HPP
#define FINANCETECHNOLOGYPROJECTS_BINANCE_EXECUTION_REPORT_SOURCE_HPP

#include <string>

#include "execution_report_source.hpp"
#include "execution_work_item.hpp"
#include "queue.hpp"


namespace trading::exchanges::binance
{
    class BinanceExecutionReportSource final: public execution::IExecutionReportSource
    {
    public:
        BinanceExecutionReportSource(std::string endpoint,
                                     concurrency::Queue<execution::ExecutionWorkItem>& executionQueue) noexcept;

        BinanceExecutionReportSource(const BinanceExecutionReportSource&) = delete;
        BinanceExecutionReportSource& operator=(const BinanceExecutionReportSource&) = delete;

        BinanceExecutionReportSource(BinanceExecutionReportSource&&) = delete;
        BinanceExecutionReportSource& operator=(BinanceExecutionReportSource&&) = delete;

        void start() override;
        void stop() override;

        void emit(const execution::ExecutionReport& report) const;

    private:
        std::string endpoint;
        concurrency::Queue<execution::ExecutionWorkItem>& executionQueue;
        bool running { false };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BINANCE_EXECUTION_REPORT_SOURCE_HPP