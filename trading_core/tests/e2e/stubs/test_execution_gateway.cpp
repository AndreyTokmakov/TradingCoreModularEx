/**============================================================================
Name        : test_execution_gateway.cpp
Created on  : 21.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Test execution gateway implementation.
============================================================================**/

#include "test_execution_gateway.hpp"

#include <utility>

namespace trading::testing::stubs
{
    TestExecutionGateway::TestExecutionGateway(std::string endpoint) noexcept :
        endpoint { std::move(endpoint) }
    {
    }

    TestExecutionGateway::TestExecutionGateway(std::string endpoint,
                                                     SendHandler sendHandler,
                                                     CancelHandler cancelHandler) noexcept :
        endpoint { std::move(endpoint) },
        sendHandler { std::move(sendHandler) },
        cancelHandler { std::move(cancelHandler) }
    {
    }

    void TestExecutionGateway::setSendHandler(SendHandler handler) noexcept
    {
        sendHandler = std::move(handler);
    }

    void TestExecutionGateway::setCancelHandler(CancelHandler handler) noexcept
    {
        cancelHandler = std::move(handler);
    }

    void TestExecutionGateway::send(const execution::Order& order)
    {
        if (!sendHandler)
            return;
        sendHandler(order);
    }

    void TestExecutionGateway::cancel(const OrderId orderId)
    {
        if (!cancelHandler)
            return;

        cancelHandler(orderId);
    }
}