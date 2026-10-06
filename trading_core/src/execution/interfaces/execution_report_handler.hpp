/**============================================================================
Name        : execution_report_handler.hpp
Created on  : 06.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : execution_report_handler.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_EXECUTION_REPORT_HANDLER_HPP
#define TRADINGCOREMODULAREX_EXECUTION_REPORT_HANDLER_HPP

#include <string_view>

namespace trading::execution
{
    struct IExecutionReportHandler
    {
        virtual ~IExecutionReportHandler() = default;
        virtual void onExecutionReport(std::string_view message) = 0;
    };
}

#endif //TRADINGCOREMODULAREX_EXECUTION_REPORT_HANDLER_HPP
