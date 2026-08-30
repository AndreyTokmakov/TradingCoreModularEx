/**============================================================================
Name        : test_execution_report_source.hpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Test execution report source.
============================================================================**/


#ifndef FINANCETECHNOLOGYPROJECTS_TEST_EXECUTION_REPORT_SOURCE_HPP
#define FINANCETECHNOLOGYPROJECTS_TEST_EXECUTION_REPORT_SOURCE_HPP

#include <string>

#include "execution_report_source.hpp"
#include "execution_work_item.hpp"
#include "queue.hpp"


namespace trading::testing::stubs
{
    class TestExecutionReportSource final: public execution::IExecutionReportSource
    {
    public:
        explicit TestExecutionReportSource(std::string endpoint,
                                           concurrency::Queue<execution::ExecutionWorkItem>& executionQueue) noexcept;

        TestExecutionReportSource(const TestExecutionReportSource&) = delete;
        TestExecutionReportSource& operator=(const TestExecutionReportSource&) = delete;

        TestExecutionReportSource(TestExecutionReportSource&&) = delete;
        TestExecutionReportSource& operator=(TestExecutionReportSource&&) = delete;

        void start() override;
        void stop() override;

        void emit(const execution::ExecutionReport& report) const;

    private:
        std::string endpoint;
        concurrency::Queue<execution::ExecutionWorkItem>& executionQueue;
        bool running { false };
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_TEST_EXECUTION_REPORT_SOURCE_HPP