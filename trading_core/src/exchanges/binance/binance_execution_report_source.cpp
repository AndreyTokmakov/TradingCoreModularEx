/**============================================================================
Name        : binance_execution_report_source.cpp
Created on  : 22.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Binance execution report source implementation.
============================================================================**/

#include "binance_execution_report_source.hpp"
#include "utils.hpp"
#include "utilities/utilities.hpp"

namespace
{
    constexpr uint16_t Port { 443 };
    constexpr std::string Target = "/ws-api/v3";
    constexpr std::int64_t RecvWindow = 5000;
}


namespace trading::exchanges::binance
{
    BinanceExecutionReportSource::BinanceExecutionReportSource(std::string endpoint,
                                                               networking::NetworkRuntime& networkRuntime) noexcept :
        BinanceExecutionReportSource { std::move(endpoint), {}, {} , networkRuntime}
    {
    }

    BinanceExecutionReportSource::BinanceExecutionReportSource(std::string endpoint,
                                                               std::string api_key,
                                                               std::string secret_key,
                                                               networking::NetworkRuntime& networkRuntime) noexcept :
        apiKey { std::move(api_key) },
        secretKey { std::move(secret_key) },
        wsClient {
            networkRuntime.getIoContext(), networkRuntime.getSslContext(),std::move(endpoint) ,std::to_string(Port), Target
        }
    {

    }

    void BinanceExecutionReportSource::setReportHandler(execution::IExecutionReportHandler& handler) {
        reportHandler = &handler;
    }

    void BinanceExecutionReportSource::start()
    {
        if (apiKey.empty() || secretKey.empty()) {
            return;
        }

        std::optional<std::string> request = createSubscribeRequest();
        if (!request.has_value()) {
            return;
        }

        wsClient.start([this](const std::string& message) {
            reportHandler->onExecutionReport(message);
        });
        wsClient.send(std::move(*request));
    }

    std::optional<std::string> BinanceExecutionReportSource::createSubscribeRequest() const
    {
        const std::int64_t timestamp = utilities::unixTimestampMilliseconds();

        // Binance requires parameters to be sorted alphabetically when calculating the signature.
        const std::string payload = std::format("apiKey={}&recvWindow={}&timestamp={}", apiKey , RecvWindow, timestamp);
        const std::optional<std::string> signature = utilities::binance::createSignature(payload, secretKey);
        if (!signature.has_value()) {
            return std::nullopt;
        }

        return R"({"id":"1","method":"userDataStream.subscribe.signature","params":{)"
               R"("apiKey":")" + apiKey +
               R"(","recvWindow":)" + std::to_string(RecvWindow) +
               R"(,"timestamp":)" + std::to_string(timestamp) +
               R"(,"signature":")" + *signature +
               R"("}})";
    }

    void BinanceExecutionReportSource::stop()
    {
        wsClient.stop();
    }
}