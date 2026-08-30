/**============================================================================
Name        : test_market_data_source.cpp
Created on  : 23.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : test_market_data_source.cpp
============================================================================**/

#include "test_market_data_source.hpp"


namespace trading::testing
{
    void TestMarketDataSource::start()
    {
        for (const auto& msgData : testMarketData)
            messageHandler->onMessage(msgData);
    }

    void TestMarketDataSource::stop(){
    }

    void TestMarketDataSource::setMessageHandler(market_data::IMarketDataMessageHandler& handler) {
        messageHandler = &handler;
    }

    void TestMarketDataSource::addTestMarketData(const std::vector<std::string>& msgData) {
        testMarketData = msgData;
    }
}