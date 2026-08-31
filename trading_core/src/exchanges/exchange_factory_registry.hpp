/**============================================================================
Name        : exchange_factory_registry.hpp
Created on  : 31.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : exchange_factory_registry.hpp
============================================================================**/

#ifndef TRADINGCOREMODULAREX_EXCHANGE_FACTORY_REGISTRY_HPP
#define TRADINGCOREMODULAREX_EXCHANGE_FACTORY_REGISTRY_HPP

#include "exchange_factory.hpp"

#include <memory>
#include <string_view>

namespace trading::exchanges
{
    struct ExchangeFactoryRegistry final
    {
        [[nodiscard]]
        static std::unique_ptr<IExchangeFactory> createFactory(std::string_view exchangeName);
    };
}


#endif //TRADINGCOREMODULAREX_EXCHANGE_FACTORY_REGISTRY_HPP
