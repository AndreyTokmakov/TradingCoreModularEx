/**============================================================================
Name        : trade_processor.hpp
Created on  : 06.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : trade_processor.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_TRADE_PROCESSOR_HPP
#define TRADINGCOREMODULAREX_TRADE_PROCESSOR_HPP

#include "market_data/model/trade.hpp"

namespace trading::trade
{
    class TradeProcessor final
    {
    public:
        explicit TradeProcessor(InstrumentId instrument) noexcept;

        void onTrade(const market_data::Trade& trade) noexcept;

        [[nodiscard]]
        const market_data::Trade& getLastTrade() const noexcept;

        [[nodiscard]]
        Quantity getBuyVolume() const noexcept;

        [[nodiscard]]
        Quantity getSellVolume() const noexcept;

        [[nodiscard]]
        uint64_t getTradeCount() const noexcept;

    private:
        InstrumentId instrument;
        market_data::Trade lastTrade {};
        Quantity buyVolume {};
        Quantity sellVolume {};
        uint64_t tradeCount { 0 };
    };
}

#endif //TRADINGCOREMODULAREX_TRADE_PROCESSOR_HPP
