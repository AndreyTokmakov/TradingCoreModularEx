/**============================================================================
Name        : CSVReader.cpp
Created on  : 03.03.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : CSVReader.h
============================================================================**/

#include "CSVReader.h"

#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <charconv>
#include <optional>
#include <filesystem>


namespace CSVReader
{
    constexpr char delimiter { ',' };
    constexpr char dQuotesSymbol { '"' };
    constexpr char sQuotesSymbol { '\'' };


    template<typename... Args>
    Value::Value(Args &&... param): data { std::forward<Args>(param)... } {
    }

    template<std::default_initializable T>
    [[nodiscard]]
    std::optional<T> Value::get() const noexcept
    {
        T value {};
        if (const auto [ptr, error_code] = std::from_chars(data.data(), data.data() + data.size(), value);
            std::errc{} != error_code) {
            return std::nullopt;
        }
        return value;
    }

    [[nodiscard]]
    int Value::asInt(int defaultValue) const noexcept
    {
        return get<int>().value_or(defaultValue);
    }

    [[nodiscard]]
    double Value::asDouble(double defaultValue) const noexcept
    {
        return get<double>().value_or(defaultValue);
    }

    std::ostream& operator<<(std::ostream& stream, const Value& val)
    {
        stream << val.data;
        return stream;
    }




    Row::Row(Headers* headersPtr): headers {headersPtr} {
    };

    [[nodiscard]]
    size_t Row::size() const noexcept {
        return values.size();
    }

    template<class ... Args>
    void Row::emplaceValue(Args&& ... params)
    {
        values.emplace_back(std::forward<Args>(params)...);
    }

    // TODO: return optional ?
    Value Row::operator[](const std::string& hdr) const
    {
        if (nullptr == headers)
            return Value{}; // TODO: std::nullopt

        if (const auto iter = headers->find(hdr); headers->end() != iter)
            return values[iter->second];

        return Value{}; // TODO: std::nullopt
    }

    Value& Row::operator[](const size_t idx)
    {
        return values[idx];
    }

    const Value& Row::operator[](const size_t idx) const
    {
        return values[idx];
    }


    [[nodiscard]]
    std::size_t CSVData::size() const noexcept {
        return rows.size();
    }

    [[nodiscard]]
    bool CSVData::hasHeaders() const noexcept
    {
        return !headers.empty();
    }

    inline size_t parseLine(const std::string& line,
                            Row& parts)
    {
        if (line.empty())
            return 0;

        bool sQuotes { false },  dQuotes { false };
        size_t prev {0}, idx {0};
        for (; idx < line.size(); ++idx)
        {
            const char ch { line[idx] };
            if (dQuotesSymbol == ch) {
                dQuotes = !dQuotes;
            } else if (sQuotesSymbol == ch) {
                sQuotes = !sQuotes;
            } else if (delimiter == ch && !dQuotes && !sQuotes) {
                parts.emplaceValue(line, prev, idx - prev);
                prev = idx + 1;
                continue;
            }
        }
        parts.emplaceValue(line, prev, idx - prev);
        return parts.size();
    }

    CSVData readCsv(const std::filesystem::path& filePath,
                    bool skipHeader)
    {
        CSVData csvData {};
        if (std::ifstream file {filePath}; file.is_open() && file.good())
        {
            std::string line;

            // Reading the header line
            if (!skipHeader)
            {
                Row header;
                if (std::getline(file, line))
                    parseLine(line, header);
                else
                    return csvData;

                for (uint16_t idx {0}; Value& hdr: header.values)
                csvData.headers[std::move(hdr.data)] = idx++;
            }

            // Read the remaining lines of the CSV file.
            while (std::getline(file, line)) {
                parseLine(line, csvData.rows.emplace_back(&csvData.headers));
            }
        }
        return csvData;
    }
}


void Value_Tests()
{
    using namespace CSVReader;

    Value val {"123.45"};
    std::cout << val.asInt() << std::endl;
    // std::cout << val.asDouble() << std::endl;
}
