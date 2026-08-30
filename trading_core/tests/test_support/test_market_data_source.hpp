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

        void addTestMarketData(const std::vector<std::string>&  msgData);

    private:
        market_data::IMarketDataMessageHandler* messageHandler { nullptr };
        std::vector<std::string> testMarketData;
    };
}


#endif //TRADINGCOREBASE_TEST_MARKET_DATA_SOURCE_HPP
