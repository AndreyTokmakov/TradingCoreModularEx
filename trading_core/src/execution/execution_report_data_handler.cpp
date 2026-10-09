/**============================================================================
Name        : execution_report_data_handler.cpp
Created on  : 06.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : execution_report_data_handler.cpp
============================================================================**/

#include "execution_report_data_handler.hpp"
#include "logging/logger_factory.hpp"

namespace trading::execution
{
    ExecutionReportDataHandler::ExecutionReportDataHandler(concurrency::Queue<ExecutionWorkItem>& executionQueue):
        executionQueue { executionQueue },
        logger { logging::LoggerFactory::getLogger() }
    {

    }

    void ExecutionReportDataHandler::onExecutionReport(std::string_view)
    {
        // TODO:
        //  - parse message
        //  - create ExecutionReport
        //  - publish ExecutionReport
    }

    // TODO: Check if we need it
    void ExecutionReportDataHandler::publish(const ExecutionReport& report) const
    {
        executionQueue.push(report);
    }
}
