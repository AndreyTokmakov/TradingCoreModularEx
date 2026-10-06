/**============================================================================
Name        : trade_processor.cpp
Created on  : 06.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : trade_processor.cpp
============================================================================**/

#include "trade_processor.hpp"

namespace trading::trade
{
    TradeProcessor::TradeProcessor(const InstrumentId instrument) noexcept :
        instrument { instrument }
    {
    }

    void TradeProcessor::onTrade(const market_data::Trade& trade) noexcept
    {
        if (trade.instrument != instrument)
            return;

        lastTrade = trade;

        if (trade.side == Side::Buy)
            buyVolume += trade.quantity;
        else
            sellVolume += trade.quantity;

        ++tradeCount;
    }

    const market_data::Trade& TradeProcessor::getLastTrade() const noexcept
    {
        return lastTrade;
    }

    Quantity TradeProcessor::getBuyVolume() const noexcept
    {
        return buyVolume;
    }

    Quantity TradeProcessor::getSellVolume() const noexcept
    {
        return sellVolume;
    }

    uint64_t TradeProcessor::getTradeCount() const noexcept
    {
        return tradeCount;
    }
}