/**============================================================================
Name        : test_execution_report_source.cpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Test execution report source implementation.
============================================================================**/

#include "test_execution_report_source.hpp"

namespace trading::testing::stubs
{
    TestExecutionReportSource::TestExecutionReportSource(std::string endpoint,
                                                         concurrency::Queue<execution::ExecutionWorkItem>& executionQueue) noexcept :
        endpoint { std::move(endpoint) },
        executionQueue { executionQueue }
    {
    }

    void TestExecutionReportSource::start()
    {
        if (running)
            return;
        running = true;

        /*
            Establish Binance execution WebSocket connection here.

            Incoming Binance messages must be parsed and converted into
            trading::execution::ExecutionReport.

            After successful normalization:

                reportHandler->onExecutionReport(report);
        */
    }


    void TestExecutionReportSource::stop()
    {
        running = false;
    }

    void TestExecutionReportSource::emit(const execution::ExecutionReport& report) const
    {
        if (!running)
            return;
        executionQueue.push(report);
    }
}