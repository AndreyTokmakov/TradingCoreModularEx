/**============================================================================
Name        : test_market_data_source.hpp
Created on  : 23.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : test_market_data_source.hpp
============================================================================**/

#ifndef TRADINGCOREBASE_TEST_MARKET_DATA_SOURCE_HPP
#define TRADINGCOREBASE_TEST_MARKET_DATA_SOURCE_HPP

#include "market_data_message_handler.hpp"
#include "interfaces/market_data_source.hpp"

namespace trading::testing
{
    class TestMarketDataSource final : public market_data::IMarketDataSource
    {
    public:
        void start() override;
        void stop() override;

        void setMessageHandler(market_data::IMarketDataMessageHandler& handler) override;
        void addTestMarketData(const std::vector<std::string>& messageData);

        [[nodiscard]]
        uint32_t startCount() const noexcept;

        [[nodiscard]]
        uint32_t stopCount() const noexcept;

        [[nodiscard]]
        bool hasMessageHandler() const noexcept;

    private:
        market_data::IMarketDataMessageHandler* messageHandler { nullptr };
        std::vector<std::string> testMarketData;
        uint32_t starts { 0 };
        uint32_t stops { 0 };
    };
}


#endif //TRADINGCOREBASE_TEST_MARKET_DATA_SOURCE_HPP
