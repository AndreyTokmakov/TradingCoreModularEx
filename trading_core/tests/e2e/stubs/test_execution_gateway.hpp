/**============================================================================
Name        : test_execution_gateway.hpp
Created on  : 21.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Test implementation of the execution gateway.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_TEST_EXECUTION_GATEWAY_HPP
#define FINANCETECHNOLOGYPROJECTS_TEST_EXECUTION_GATEWAY_HPP

#include "execution_gateway.hpp"

#include <functional>
#include <string>

namespace trading::testing::stubs
{
    class TestExecutionGateway final : public execution::IExecutionGateway
    {
    public:
        using SendHandler   = std::function<void(const execution::Order&)>;
        using CancelHandler = std::function<void(OrderId)>;

        explicit TestExecutionGateway(std::string endpoint) noexcept;

        TestExecutionGateway(std::string endpoint,
                                SendHandler sendHandler,
                                CancelHandler cancelHandler) noexcept;

        void setSendHandler(SendHandler sendHandler) noexcept;

        void setCancelHandler(CancelHandler cancelHandler) noexcept;

        void send(const execution::Order& order) override;

        void cancel(OrderId orderId) override;

    private:
        std::string endpoint;
        SendHandler sendHandler;
        CancelHandler cancelHandler;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_TEST_EXECUTION_GATEWAY_HPP