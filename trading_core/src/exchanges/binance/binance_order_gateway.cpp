/**============================================================================
Name        : binance_execution_gateway.cpp
Created on  : 21.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance execution gateway implementation.
============================================================================**/

#include "binance_order_gateway.hpp"

#include <utility>
#include <iomanip>
#include <format>
#include "utils.hpp"
#include "utilities/utilities.hpp"

namespace
{
    using namespace trading::exchanges::binance;

    using trading::Price;
    using trading::Quantity;
    using trading::Side;
    using trading::OrderId;
    using trading::OrderType;

    [[nodiscard]]
    std::string toDecimalString(const std::int64_t value, const int decimalPlaces)
    {
        const bool negative = value < 0;

        // Avoid overflow for INT64_MIN.
        const std::uint64_t absoluteValue = negative ? static_cast<std::uint64_t>(-(value + 1)) + 1
            : static_cast<std::uint64_t>(value);

        std::uint64_t scale = 1;
        for (int index = 0; index < decimalPlaces; ++index) {
            scale *= 10;
        }

        const std::uint64_t integerPart = absoluteValue / scale;
        const std::uint64_t fractionalPart = absoluteValue % scale;

        std::ostringstream stream;
        if (negative) {
            stream << '-';
        }

        stream << integerPart;

        if (fractionalPart != 0) {
            stream << '.' << std::setfill('0') << std::setw(decimalPlaces) << fractionalPart;
            std::string result = stream.str();
            while (result.back() == '0') {
                result.pop_back();
            }
            if (result.back() == '.') {
                result.pop_back();
            }
            return result;
        }

        return stream.str();
    }

    [[nodiscard]]
    std::string toString(const Price& price)
    {
        return toDecimalString(price.raw(), Price::DecimalPlaces);
    }

    [[nodiscard]]
    std::string toString(const Quantity& quantity)
    {
        return toDecimalString(quantity.raw(), Quantity::DecimalPlaces);
    }

    [[nodiscard]]
    std::string createOrderQuery(const trading::execution::Order& order,
                                 const std::string_view symbol)
    {
        std::string query = std::format("symbol={}&side={}&type={}&quantity={}&newClientOrderId={}",
            trading::utilities::urlEncode(symbol),
            trading::toString(order.side),
            trading::toString(order.type),
            toString(order.quantity),
            order.clientOrderId
        );

        if (order.type == OrderType::Limit) {
            query += std::format("&price={}&timeInForce=GTC",toString(order.price));
        }

        return query;
    }

    [[nodiscard]]
    BinanceApiError makeError(const BinanceApiErrorCode code,
                              std::string message,
                              const std::int64_t exchangeErrorCode = 0)
    {
        return BinanceApiError {
            .code = code,
            .exchangeErrorCode = exchangeErrorCode,
            .message = std::move(message)
        };
    }

    constexpr std::string ApiPath = "/api/v3/order";
}

namespace trading::exchanges::binance
{

    BinanceOrderGateway::BinanceOrderGateway(std::string endpoint,
                                             std::string api_key,
                                             std::string secret_keys,
                                             std::string symbol,
                                             const std::uint64_t recvWindow):
        endpoint { std::move(endpoint) },
        ioContext { 1 },
        sslContext { boost::asio::ssl::context::tls_client },
        restClient { ioContext, sslContext },
        apiKey { std::move(api_key) },
        secretKey { std::move(secret_keys) },
        symbol { std::move(symbol) },
        recvWindow { recvWindow }
    {

    }

    void BinanceOrderGateway::asyncSignedRequest(const boost::beast::http::verb method,
                                           std::string query,
                                           ApiResponseHandler handler)
    {
        if (!handler) {
            return;
        }
        if (apiKey.empty() || secretKey.empty()) {
            handler(std::unexpected {makeError(BinanceApiErrorCode::SigningFailed, "Binance API credentials are empty")});
            return;
        }
        if (!query.empty()) {
            query += '&';
        }

        query += std::format("recvWindow={}&timestamp={}", recvWindow, utilities::unixTimestampMilliseconds());
        const std::optional<std::string> signature = utilities::binance::createSignature(query, secretKey);
        if (!signature) {
            handler(std::unexpected { makeError(BinanceApiErrorCode::SigningFailed, "Failed to create HMAC SHA256 signature")});
            return;
        }

        network::HttpRequest httpRequest {
            .host = ApiPath,
            .target = std::format("{}?{}&signature={}", endpoint, query, signature.value()),
            .method = method,
            .headers = {
                    { "X-MBX-APIKEY", apiKey }
            }
        };

        restClient.asyncRequest(httpRequest,[handler = std::move(handler)](RestResponse response) mutable
        {
            if (!response) {
                handler(std::unexpected {makeError(BinanceApiErrorCode::TransportError,response.error().message)});
                return;
            }
            handler(std::move(*response));
        });
    }


    void BinanceOrderGateway::send(const execution::Order& order)
    {
        const std::string_view side = toString(order.side);
        const std::string_view type = toString(order.type);

        if (order.clientOrderId == 0 ||
            order.quantity.raw() <= 0 ||
            side.empty() ||
            type.empty() ||
            (order.type == OrderType::Limit && order.price.raw() <= 0))
        {
            // handler(std::unexpected {makeError(BinanceApiErrorCode::InvalidRequest,"Invalid order parameters")});
            return;
        }

        if (symbol.empty()) {
            // handler(std::unexpected {makeError(BinanceApiErrorCode::InvalidRequest,"Instrument symbol is empty")});
            return;
        }

        const std::string query = createOrderQuery(order, symbol);
    }

    void BinanceOrderGateway::cancel(ExchangeOrderId)
    {

    }
}