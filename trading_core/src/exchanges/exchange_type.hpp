/**============================================================================
Name        : exchange_type.hpp
Created on  : 01.09.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description :
    Defines the list of supported exchange types.

    ExchangeType provides a strongly typed identifier for an exchange and is
    used instead of string-based exchange names when selecting an exchange
    implementation.

    The type is primarily used by ExchangeFactoryRegistry to create the
    corresponding IExchangeFactory implementation. The returned factory then
    creates exchange-specific components such as:

        - IMarketDataSource
        - IMarketDataParser
        - ISnapshotProvider
        - IExecutionGateway
        - IExecutionReportSource

    When support for a new exchange is added, a new value should be added to
    ExchangeType and ExchangeFactoryRegistry should be extended to create the
    corresponding exchange factory.

    Location:
        src/exchanges/exchange_type.hpp

    Example usage:

        const auto exchangeFactory =
            ExchangeFactoryRegistry::createFactory(ExchangeType::Binance);
============================================================================**/

#ifndef TRADINGCOREMODULAREX_EXCHANGE_TYPE_HPP
#define TRADINGCOREMODULAREX_EXCHANGE_TYPE_HPP

#include <cstdint>

namespace trading::exchanges
{
    enum class ExchangeType : uint8_t
    {
        Binance
    };
}

#endif //TRADINGCOREMODULAREX_EXCHANGE_TYPE_HPP