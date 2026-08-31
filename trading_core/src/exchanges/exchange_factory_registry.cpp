/**============================================================================
Name        : exchange_factory_registry.cpp
Created on  : 31.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : exchange_factory_registry.cpp
============================================================================**/

#include "exchange_factory_registry.hpp"

#include "binance_exchange_factory.hpp"
#include <stdexcept>

namespace trading::exchanges
{
    std::unique_ptr<IExchangeFactory>
    ExchangeFactoryRegistry::createFactory(const std::string_view exchangeName)
    {
        if ("Binance" == exchangeName)
            return std::make_unique<binance::BinanceExchangeFactory>();

        throw std::invalid_argument {
            "Unsupported exchange: " + std::string { exchangeName }
        };
    }
}