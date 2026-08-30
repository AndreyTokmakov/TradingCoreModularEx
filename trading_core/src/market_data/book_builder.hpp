/**============================================================================
Name        : book_builder.hpp
Created on  : 16.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Order book builder.
============================================================================**/

/*
    BookBuilder is responsible for applying normalized market-data events to
    an OrderBook and publishing MarketEvent objects after successfully
    updating the book.

    BookBuilder sits between normalized market data and consumers interested
    in the resulting order-book state.

    Data flow:

        Snapshot
           |
           v
        BookBuilder::applySnapshot()
           |
           v
        OrderBook
           |
           v
        MarketEvent


        BookUpdate
           |
           v
        BookBuilder::onBookUpdate()
           |
           v
        OrderBook
           |
           v
        MarketEvent


    Responsibilities:

        - receive a complete Snapshot;
        - receive incremental BookUpdate events;
        - verify that updates belong to the configured instrument;
        - apply snapshots to OrderBook;
        - apply incremental updates to OrderBook;
        - obtain the resulting best bid and best ask;
        - create and publish MarketEvent objects.


    Snapshot processing:

        A Snapshot establishes the complete initial state of the OrderBook.

        Snapshot
            |
            v
        OrderBook::replace()
            |
            v
        valid OrderBook
            |
            v
        MarketEvent


    Incremental update processing:

        BookUpdate
            |
            v
        OrderBook::applyUpdate()
            |
            v
        updated OrderBook
            |
            v
        MarketEvent


    BookBuilder does not:

        - parse exchange-specific market-data messages;
        - communicate with a market-data source;
        - know about exchange-specific protocols;
        - create Orders;
        - perform risk checks;
        - execute trading strategies;
        - communicate with an execution gateway.


    Market-data parsing is performed by IMarketDataParser and exchange-specific
    parser implementations.

    OrderBook owns the actual current state of the local order book.

    BookBuilder is therefore an orchestration component connecting:

        normalized market data
                |
                v
           OrderBook
                |
                v
          MarketEvent
*/

#ifndef FINANCETECHNOLOGYPROJECTS_BOOK_BUILDER_HPP
#define FINANCETECHNOLOGYPROJECTS_BOOK_BUILDER_HPP

#include "interfaces/book_update_handler.hpp"
#include "interfaces/market_event_handler.hpp"
#include "model/snapshot.hpp"
#include "order_book.hpp"

namespace trading::market_data
{
    class BookBuilder final : public IBookUpdateHandler
    {
    public:
        BookBuilder(InstrumentId instrument,
                    OrderBook& orderBook,
                    IMarketEventHandler& eventHandler) noexcept;

        [[nodiscard]]
        bool applySnapshot(const Snapshot& snapshot) const;

        void onBookUpdate(const BookUpdate& update) override;

    private:
        void publishMarketEvent(SequenceNumber sequence,
                                Timestamp exchangeTimestamp) const;

        InstrumentId instrument;
        OrderBook& orderBook;
        IMarketEventHandler& eventHandler;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_BOOK_BUILDER_HPP