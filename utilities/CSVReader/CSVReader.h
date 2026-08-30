/**============================================================================
Name        : CSVReader.h
Created on  : 03.03.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : CSVReader.h
============================================================================**/

#ifndef CPPPROJECTS_CSVREADER_H
#define CPPPROJECTS_CSVREADER_H

#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <charconv>
#include <optional>
#include <filesystem>

namespace CSVReader
{
    using Headers = std::map<std::string, uint16_t>;

    struct Value
    {
        std::string data {};

        template<typename ... Args>
        explicit Value(Args&& ... param);

        template<std::default_initializable T>
        [[nodiscard]]
        std::optional<T> get() const noexcept;

        [[nodiscard]]
        int asInt(int defaultValue = 0) const noexcept;

        [[nodiscard]]
        double asDouble(double defaultValue = 0.0f) const noexcept;

        friend std::ostream& operator<<(std::ostream& stream, const Value& val);
    };


    struct Row
    {
        std::vector<Value> values {};
        Headers* headers { nullptr };

        explicit Row(Headers* headersPtr = nullptr);

        [[nodiscard]]
        size_t size() const noexcept;

        template<class ... Args>
        void emplaceValue(Args&& ... params);

        // TODO: return optional ?
        Value operator[](const std::string& hdr) const;

        Value& operator[](const size_t idx);

        const Value& operator[](const size_t idx) const;
    };


    struct CSVData
    {
        Headers headers;
        std::vector<Row> rows;

        [[nodiscard]]
        std::size_t size() const noexcept;

        [[nodiscard]]
        bool hasHeaders() const noexcept;
    };

    CSVData readCsv(const std::filesystem::path& filePath,
                    bool skipHeader = false);
};

#endif //CPPPROJECTS_CSVREADER_H
