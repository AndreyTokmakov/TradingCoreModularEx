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

#include <simdjson.h>

namespace
{
    using namespace trading::exchanges::binance;

    using trading::Price;
    using trading::Quantity;
    using trading::Side;
    using trading::OrderId;
    using trading::OrderType;
    using trading::network::HttpResponse;

    constexpr std::string_view OrderEndpoint = "/api/v3/order";
    constexpr uint16_t port = 433;

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
    bool getJsonInt64(const simdjson::dom::element& json,
                      const std::string_view key,
                      std::int64_t& output)
    {
        if (!json[key].get_int64().get(output)) {
            return true;
        }
        return false;
    }

    [[nodiscard]]
    bool getJsonString(const simdjson::dom::element& json,
                       const std::string_view key,
                       std::string& output)
    {
        std::string_view value;
        if (json[key].get_string().get(value)) {
            return false;
        }

        output.assign(value);
        return true;
    }

    [[nodiscard]]
    bool getJsonUint64(const simdjson::dom::element& json,
                       const std::string_view key,
                       std::uint64_t& output)
    {
        if (!json[key].get_uint64().get(output)) {
            return true;
        }
        if (std::int64_t signedValue = 0; !json[key].get_int64().get(signedValue) && signedValue >= 0) {
            output = static_cast<std::uint64_t>(signedValue);
            return true;
        }

        return false;
    }

    [[nodiscard]]
    std::string createOrderQuery(const Order& order,
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
    std::unexpected<OrderGatewayError> makeError(const ErrorCode code,
                                               std::string message,
                                               const std::int64_t exchangeErrorCode = 0)
    {
        return std::unexpected<OrderGatewayError>(OrderGatewayError {
            .code = code,
            .exchangeErrorCode = exchangeErrorCode,
            .message = std::move(message)
        });
    }

    [[nodiscard]]
    BinanceOrderGateway::OrderResponse parseOrderResponse(const HttpResponse& response,
                                                          const OrderId clientOrderId)
    {
        simdjson::dom::parser parser;
        simdjson::dom::element json;

        const simdjson::error_code errorCode = parser.parse(response.body).get(json);
        if (errorCode != simdjson::SUCCESS || !json.is_object()) {
            return std::unexpected {makeError(ErrorCode::InvalidResponse, "Invalid Binance order response")
            };
        }

        if (response.status < 200 || response.status >= 300)
        {
            std::int64_t exchangeErrorCode = 0;
            std::string message;

            if (!getJsonInt64(json, "code", exchangeErrorCode)) {
                return std::unexpected {makeError(ErrorCode::InvalidResponse,"Failed to parse error code")};
            }
            if (!getJsonString(json, "msg", message)) {
                return std::unexpected {makeError(ErrorCode::InvalidResponse,"Failed to parse error message")};
            }

            return std::unexpected { makeError(ErrorCode::ExchangeRejected,
                message.empty() ? "Binance rejected order" : std::move(message), exchangeErrorCode)
            };
        }

        std::uint64_t exchangeOrderId = 0;
        std::string status;

        if (!getJsonUint64(json, "orderId", exchangeOrderId) || !getJsonString(json, "status", status) || exchangeOrderId == 0)
        {
            return std::unexpected {makeError(ErrorCode::InvalidResponse,
                "Binance response does not contain orderId or status")};
        }

        const trading::OrderStatus orderStatus = trading::toOrderStatus(status);
        return OrderSubmissionResult {
            .clientOrderId = clientOrderId,
            .exchangeOrderId = exchangeOrderId,
            .status = orderStatus
        };
    }

    [[nodiscard]]
    BinanceOrderGateway::CancelResponse parseCancelResponse(const HttpResponse& response)
    {
        simdjson::dom::parser parser;
        simdjson::dom::element json;

        const simdjson::error_code errorCode = parser.parse(response.body).get(json);
        if (errorCode != simdjson::SUCCESS || !json.is_object()) {
            return std::unexpected { makeError(ErrorCode::InvalidResponse,"Invalid Binance cancel response")};
        }

        if (response.status < 200 || response.status >= 300)
        {
            std::int64_t exchangeErrorCode = 0;
            std::string message;

            if (!getJsonInt64(json, "code", exchangeErrorCode)) {
                return std::unexpected {makeError(ErrorCode::InvalidResponse,"Failed to parse error code")};
            }
            if (!getJsonString(json, "msg", message)) {
                return std::unexpected {makeError(ErrorCode::InvalidResponse,"Failed to parse error message")};
            }

            return std::unexpected { makeError(ErrorCode::ExchangeRejected,
                message.empty() ? "Binance rejected cancellation" : std::move(message), exchangeErrorCode)
            };
        }

        std::uint64_t exchangeOrderId = 0;
        std::string status;

        if (!getJsonUint64(json, "orderId", exchangeOrderId) ||
            !getJsonString(json, "status", status) || exchangeOrderId == 0)
        {
            return std::unexpected {makeError(ErrorCode::InvalidResponse,
                          "Binance cancel response does not contain orderId or status")
            };
        }

        const trading::OrderStatus orderStatus = trading::toOrderStatus(status);
        return OrderCancellationResult {
            .exchangeOrderId = exchangeOrderId,
            .status = orderStatus
        };
    }
}

namespace trading::exchanges::binance
{
    BinanceOrderGateway::BinanceOrderGateway(std::string endpoint,
                                             std::string api_key,
                                             std::string secret_keys,
                                             networking::NetworkRuntime& networkRuntime,
                                             std::string symbol,
                                             const std::uint64_t recvWindow):
        endpoint { std::move(endpoint) },
        restClient { networkRuntime.getIoContext(), networkRuntime.getSslContext() },
        apiKey { std::move(api_key) },
        secretKey { std::move(secret_keys) },
        symbol { std::move(symbol) },
        recvWindow { recvWindow }
    {

    }

    void BinanceOrderGateway::send(const Order&)
    {
    }

    void BinanceOrderGateway::cancel(ExchangeOrderId)
    {
    }

    void BinanceOrderGateway::sendAsync(const Order& order,
                                        OrderId clientOrderId,
                                        OrderHandler handler)
    {
        if (!handler) {
            return;
        }

        const std::string_view side = toString(order.side);
        const std::string_view type = toString(order.type);

        if (clientOrderId == 0 || order.quantity.raw() <= 0 || side.empty() || type.empty() ||
            (order.type == OrderType::Limit && order.price.raw() <= 0))
        {
            handler(makeError(ErrorCode::InvalidRequest,"Invalid order parameters"));
            return;
        }

        if (symbol.empty()) {
            handler( makeError(ErrorCode::InvalidRequest,"Instrument symbol is empty"));
            return;
        }

        const std::string query = createOrderQuery(order, symbol);
        asyncSignedRequest(boost::beast::http::verb::post ,query,
            [handler = std::move(handler), clientOrderId](ApiResponse response) mutable
        {
            if (!response) {
                handler(std::unexpected { response.error() });
                return;
            }
            handler(parseOrderResponse(*response, clientOrderId));
        });
    }

    void BinanceOrderGateway::cancelAsync(ExchangeOrderId exchangeOrderId,
                                           CancelHandler handler)
    {
        if (!handler) {
            return;
        }
        if (exchangeOrderId == 0) {
            handler(makeError(ErrorCode::InvalidRequest, "Invalid exchange order ID"));
            return;
        }

        const std::string query = std::format("symbol={}&orderId={}",
            utilities::urlEncode(symbol),
            exchangeOrderId
        );

        asyncSignedRequest(boost::beast::http::verb::delete_,query,
            [handler = std::move(handler)](ApiResponse response) mutable
        {
            if (!response) {
                handler(std::unexpected { response.error() });
                return;
            }
            handler(parseCancelResponse(*response));
        });
    }

    void BinanceOrderGateway::asyncSignedRequest(const boost::beast::http::verb method,
                                             std::string query,
                                             ApiResponseHandler handler)
    {
        if (!handler) {
            return;
        }
        if (apiKey.empty() || secretKey.empty()) {
            handler(makeError(ErrorCode::SigningFailed, "Binance API credentials are empty"));
            return;
        }

        if (!query.empty()) {
            query += '&';
        }

        query += std::format("recvWindow={}&timestamp={}", recvWindow, utilities::unixTimestampMilliseconds());

        const std::optional<std::string> signature = utilities::binance::createSignature(query, secretKey);
        if (!signature.has_value()) {
            handler(makeError(ErrorCode::SigningFailed, "Failed to create HMAC SHA256 signature"));
            return;
        }

        network::HttpRequest httpRequest {
            .host = endpoint,
            .port = std::to_string(port),
            .target = std::format("{}?{}&signature={}", OrderEndpoint, query, signature.value()),
            .method = method,
            .headers = {{ "X-MBX-APIKEY", apiKey }}
        };

        restClient.asyncRequest(httpRequest,[handler = std::move(handler)](RestResponse response) mutable
        {
            if (!response) {
                handler(makeError(ErrorCode::TransportError,response.error().message));
                return;
            }
            handler(std::move(*response));
        });
    }
}