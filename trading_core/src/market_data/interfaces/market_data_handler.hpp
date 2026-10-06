/**============================================================================
Name        : market_data_handler.hpp
Created on  : 06.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : market_data_handler.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_MARKET_DATA_HANDLER_HPP
#define TRADINGCOREMODULAREX_MARKET_DATA_HANDLER_HPP

#include <string_view>

namespace trading::market_data
{
    struct IMarketDataMessageHandler
    {
        virtual ~IMarketDataMessageHandler() = default;
        virtual void onMessage(std::string_view message) = 0;
    };
}

#endif //TRADINGCOREMODULAREX_MARKET_DATA_HANDLER_HPP
