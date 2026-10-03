/**============================================================================
Name        : trade.hpp
Created on  : 03.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Normalized market data event representing an executed trade.
============================================================================**/

#ifndef TRADINGCOREMODULAREX_TRADE_HPP
#define TRADINGCOREMODULAREX_TRADE_HPP

#include "core/price.hpp"
#include "core/quantity.hpp"
#include "core/timestamp.hpp"
#include "core/types.hpp"

namespace trading::market_data
{
    /*
    Trade represents a single executed trade reported by an exchange.

    Unlike BookUpdate, which describes a change to the current order book,
    Trade represents an actual execution that occurred in the market.

    Trade is a normalized internal representation. Exchange-specific trade
    messages should be converted to Trade by the corresponding market data
    parser before being passed to the rest of the trading system.

    Typical data flow:

        Exchange
            |
            | exchange-specific trade message
            v
        MarketDataSource
            |
            v
        MarketDataParser
            |
            | Trade
            v
        MarketDataProcessor
            |
            +----> Strategy
            |
            +----> Recording
            |
            +----> Metrics / Analytics

    A trade may be generated independently from an order book update.
    Therefore, Trade and BookUpdate represent different market data streams.

    Fields:

        instrument
            Identifier of the financial instrument for which the trade
            was executed.

        sequence
            Sequence number assigned by the exchange to this market data
            event. It is used to preserve ordering and detect gaps when
            the exchange provides sequence numbers for trade messages.

        exchangeTimestamp
            Timestamp assigned by the exchange when the trade occurred.

        price
            Execution price of the trade.

        quantity
            Executed quantity.

        side
            Aggressor side of the trade, when provided by the exchange.

            Side::Buy
                The trade was initiated by a buy aggressor.

            Side::Sell
                The trade was initiated by a sell aggressor.

    Not every exchange provides the aggressor side explicitly. Some
    exchanges expose buyer/seller information, while others provide a
    trade direction or require the direction to be inferred from other
    market data.

    The normalized Trade structure intentionally contains only information
    that is commonly available and useful for the MVP. Exchange-specific
    fields such as trade ID, buyer order ID, seller order ID, or execution
    flags can be added later if required by a particular exchange or
    strategy.
    */

    struct Trade
    {
        InstrumentId instrument { 0 };
        SequenceNumber sequence { 0 };
        Timestamp exchangeTimestamp {};
        Price price {};
        Quantity quantity {};
        Side side { Side::Buy };
    };
}

#endif //TRADINGCOREMODULAREX_TRADE_HPP