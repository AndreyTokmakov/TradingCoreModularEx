/**============================================================================
Name        : snapshot.hpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Order book snapshot.
============================================================================**/

/*
    Snapshot represents a complete state of an OrderBook received from an
    external market-data source.

    Unlike BookUpdate, which describes a single change to an existing order
    book, Snapshot represents the complete set of bid and ask levels required
    to initialize or resynchronize the local OrderBook.

    Conceptually:

        Snapshot
            |
            +-- instrument
            +-- sequence
            +-- exchangeTimestamp
            +-- bids
            +-- asks

    A snapshot is normally used at the beginning of market-data processing or
    when the local OrderBook needs to be resynchronized.

    --------------------------------------------------------------------------
    Snapshot vs BookUpdate
    --------------------------------------------------------------------------

    BookUpdate represents an incremental change:

        "price 60000 on the bid side now has quantity 100"

    Snapshot represents the complete current state:

        bids:
            60000 -> 100
            59999 -> 250
            ...

        asks:
            60001 -> 120
            60002 -> 300
            ...

    Therefore:

        Snapshot
            -> establishes the initial OrderBook state

        BookUpdate
            -> modifies the existing OrderBook state

    --------------------------------------------------------------------------
    Instrument
    --------------------------------------------------------------------------

    instrument identifies the instrument to which the snapshot belongs.

    This allows BookBuilder to verify that the snapshot belongs to the
    instrument for which it is responsible.

    --------------------------------------------------------------------------
    Sequence
    --------------------------------------------------------------------------

    sequence identifies the position of the snapshot in the exchange
    market-data stream.

    After a snapshot has been successfully applied, subsequent incremental
    BookUpdates can be validated against this sequence number.

    The exact sequence semantics are exchange-specific. Snapshot therefore
    stores the normalized SequenceNumber type and does not expose any
    exchange-specific protocol details.

    --------------------------------------------------------------------------
    Exchange timestamp
    --------------------------------------------------------------------------

    exchangeTimestamp represents the timestamp associated with the snapshot
    by the exchange.

    The timestamp is preserved so that downstream components can associate
    the resulting MarketEvent with the corresponding exchange time.

    --------------------------------------------------------------------------
    Ownership
    --------------------------------------------------------------------------

    Snapshot owns its bid and ask level containers.

    This makes Snapshot a self-contained domain object containing:

        - instrument;
        - sequence;
        - exchange timestamp;
        - complete bid state;
        - complete ask state.

    Snapshot does not depend on OrderBook, BookBuilder or any exchange-specific
    component.

    --------------------------------------------------------------------------
    Design
    --------------------------------------------------------------------------

    OrderBookLevels is used as the normalized representation of price levels.

    Snapshot therefore depends only on the fundamental market-data types:

        Price
        Quantity
        InstrumentId
        SequenceNumber
        Timestamp

    It does not contain processing logic.

    Snapshot does not:

        - apply itself to an OrderBook;
        - validate sequence continuity;
        - generate MarketEvent;
        - communicate with an exchange;
        - know about MarketDataSource;
        - know about BookBuilder;
        - contain exchange-specific fields.

    Applying a Snapshot remains the responsibility of BookBuilder.

    The resulting flow is:

        MarketDataSource
              |
              v
        MarketDataParser
              |
              v
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
*/

#ifndef FINANCETECHNOLOGYPROJECTS_SNAPSHOT_HPP
#define FINANCETECHNOLOGYPROJECTS_SNAPSHOT_HPP

#include "order_book_levels.hpp"
#include "timestamp.hpp"
#include "types.hpp"

namespace trading::market_data
{
    struct Snapshot
    {
        InstrumentId instrument {};
        SequenceNumber sequence {};
        Timestamp exchangeTimestamp {};

        OrderBookLevels bids;
        OrderBookLevels asks;
    };
}

#endif //FINANCETECHNOLOGYPROJECTS_SNAPSHOT_HPP