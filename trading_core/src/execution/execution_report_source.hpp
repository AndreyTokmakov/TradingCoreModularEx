/**============================================================================
Name        : execution_report_source.hpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Execution report source interface.
============================================================================**/

/*
    IExecutionReportSource represents the inbound execution boundary between
    an external execution venue and the trading core.

    Data Flow:

        Exchange / Execution Venue
                  |
                  | execution message
                  v
        Exchange-specific adapter
                  |
                  | ExecutionReport
                  v
        IExecutionReportSource
                  |
                  v
        ExecutionReportHandler

    The interface represents the opposite direction to IExecutionGateway.

    IExecutionGateway:
        Trading Core -> Exchange

    IExecutionReportSource:
        Exchange -> Trading Core

    IExecutionReportSource does not:
        - create orders;
        - send orders;
        - cancel orders;
        - modify Order;
        - modify Position;
        - perform risk checks;
        - parse exchange-specific messages.

    Exchange-specific implementations are responsible for receiving external
    execution events and converting them into normalized ExecutionReport
    objects before forwarding them to the configured handler.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_EXECUTION_REPORT_SOURCE_HPP
#define FINANCETECHNOLOGYPROJECTS_EXECUTION_REPORT_SOURCE_HPP

#include "execution_report.hpp"

namespace trading::execution
{
    struct IExecutionReportSource
    {
        virtual ~IExecutionReportSource() = default;

        virtual void start() = 0;
        virtual void stop() = 0;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_EXECUTION_REPORT_SOURCE_HPP