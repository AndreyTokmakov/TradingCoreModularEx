/**============================================================================
Name        : book_update.hpp
Created on  : 15.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Market data update representing a single order book level change.
============================================================================**/

/*
    BookUpdate represents a single incremental change to an OrderBook.

    It describes a change to one price level on either the bid or ask side.
    A zero quantity means that the corresponding price level must be removed from the OrderBook.

    BookUpdate is a normalized internal representation of an incremental market data update.
    Exchange-specific market data formats should be converted to BookUpdate by the corresponding
    market data parser before being passed to the BookBuilder.

    Data Flow:

        Exchange
           |
           | exchange-specific market data
           v
        MarketDataGateway
           |
           v
        Parser
           |
           | BookUpdate
           v
        BookBuilder
           |
           v
        OrderBook
           |
           | updated top-of-book
           v
        MarketEvent
           |
           v
        Strategy

    BookUpdate is created by the market data layer after an exchange-specific
    message has been parsed and normalized.

    BookUpdate is consumed by BookBuilder, which validates and applies the
    update to the corresponding OrderBook.

    If the update cannot be applied, for example because the sequence number is not the expected
    next sequence, BookBuilder does not produce a MarketEvent for that update.

    Fields:
        instrument
            Identifier of the financial instrument whose order book is being updated.

        sequence
            Sequence number assigned to the market data update. It identifies  the position of this  update
            in the ordered market data stream and is used by OrderBook to detect missing or out-of-order updates.

        side
            Order book side affected by the update:
                Side::Buy  - bid side
                Side::Sell - ask side

        price
            Price level affected by the update.

        quantity
            New quantity available at the specified price level.
            A non-zero quantity creates a new price level or replaces the existing quantity at that level.
            A zero quantity removes the price level from the OrderBook.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_BOOK_UPDATE_HPP
#define FINANCETECHNOLOGYPROJECTS_BOOK_UPDATE_HPP

#include "core/timestamp.hpp"
#include "core/types.hpp"

#include "sequence_range.hpp"
#include "book_level_update.hpp"

namespace trading::market_data
{
    struct BookUpdate
    {
        InstrumentId instrument { 0 };
        SequenceRange sequenceRange { 0 };
        Timestamp exchangeTimestamp {};
        std::vector<PriceLevelUpdate> updates;

        void clear()
        {
            instrument = InstrumentId{ 0 };
            sequenceRange = SequenceRange { .first = 0, .last = 0 };
            exchangeTimestamp = Timestamp { 0 };
            updates.clear();
        }

        [[nodiscard]]
        bool empty() const noexcept {
            return updates.empty();
        }
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BOOK_UPDATE_HPP