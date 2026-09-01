/**============================================================================
Name        : exchange_factory_registry.cpp
Created on  : 31.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : exchange_factory_registry.cpp
============================================================================**/

#include "exchange_type.hpp"
#include "exchange_factory_registry.hpp"

#include "binance_exchange_factory.hpp"
#include <stdexcept>

namespace trading::exchanges
{
    [[nodiscard]]
    static constexpr std::string_view toString(const ExchangeType exchangeType) noexcept
    {
        switch (exchangeType)
        {
            case ExchangeType::Binance:
                return "Binance";
        }
        return "Unknown";
    }


    std::unique_ptr<IExchangeFactory>
    ExchangeFactoryRegistry::createFactory(const ExchangeType exchangeType)
    {
        if (ExchangeType::Binance == exchangeType)
            return std::make_unique<binance::BinanceExchangeFactory>();


        throw std::invalid_argument {
            "Unsupported exchange: " + std::string { toString(exchangeType) }
        };
    }
}