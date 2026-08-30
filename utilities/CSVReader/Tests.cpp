/**============================================================================
Name        : Tests.cpp
Created on  : 03.03.2024
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : CSV Tests
============================================================================**/

#include "CSVReader.h"
#include "Tests.h"

#include <source_location>
#include <filesystem>

namespace CSV_Reader_Tests
{
    using namespace CSVReader;

    void Test_ParseFile()
    {
        constexpr std::string_view csvFile { R"(../../Utilities/data/anime.csv)"};
        CSVData data = readCsv(csvFile);

        // for (const auto& [hdr, idx]: data.headers)
        //    std::cout << hdr << " = " << idx << std::endl;

        for (const Row& row: data.rows)
        {
            std::cout << row["anime_id"].asInt() << "    " <<  row["name"] << "  " << row["episodes"] << std::endl;
        }
    }
}


void CSV_Reader_Tests::TestAll()
{
    // CSV_Reader_Tests::Test_ParseLine();
    CSV_Reader_Tests::Test_ParseFile();
    // CSV_Reader_Tests::Value_Tests();

    /*
    std::cout << std::source_location::current().file_name() << '\n'; // requires C++20
    std::cout << __FILE__ << '\n';

    std::cout << std::filesystem::current_path() << '\n';
    std::cout << std::filesystem::path(__FILE__).remove_filename() << '\n';
    */
}