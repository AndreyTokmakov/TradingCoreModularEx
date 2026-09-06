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
        ++starts;
        if (messageHandler == nullptr)
            return;

        for (const std::string& message : testMarketData) {
            messageHandler->onMessage(message);
        }
    }

    void TestMarketDataSource::stop()
    {
        ++stops;
    }

    void TestMarketDataSource::setMessageHandler(market_data::IMarketDataMessageHandler& handler)
    {
        messageHandler = &handler;
    }

    void TestMarketDataSource::addTestMarketData(const std::vector<std::string>& messageData)
    {
        testMarketData.insert(testMarketData.end(),messageData.begin(),messageData.end());
    }

    uint32_t TestMarketDataSource::startCount() const noexcept
    {
        return starts;
    }

    uint32_t TestMarketDataSource::stopCount() const noexcept
    {
        return stops;
    }

    bool TestMarketDataSource::hasMessageHandler() const noexcept
    {
        return messageHandler != nullptr;
    }
}