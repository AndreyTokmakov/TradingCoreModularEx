/**============================================================================
Name        : trade_module.hpp
Created on  : 06.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : trade_module.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_TRADE_MODULE_HPP
#define TRADINGCOREMODULAREX_TRADE_MODULE_HPP

#include "trade_processor.hpp"

#include "common/common.hpp"
#include "config/config.hpp"
#include "market_data/model/trade.hpp"

namespace trading::trade
{
    using market_data::Trade;

    class TradeModule final : public common::Worker<TradeModule>
    {
    public:
        TradeModule(const config::Config& config,
                    common::Queue<Trade>& tradeQueue) noexcept;

        void run();

        [[nodiscard]]
        const TradeProcessor& processor() const noexcept;

    private:
        common::Queue<Trade>& tradeQueue;
        TradeProcessor tradeProcessor;
    };
}

#endif //TRADINGCOREMODULAREX_TRADE_MODULE_HPP
