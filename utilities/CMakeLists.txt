cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(utilities)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pthread")
set(PROJECT_VERSION 1.0.0.0)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

add_compile_options(-c -Wall -Wextra -O3 -std=c++2c)

add_library(utils
        src/StringUtilities.cpp
        src/FileUtilities.cpp
        CSVReader/CSVReader.cpp
        CSVReader/Tests.cpp
        src/Base64.cpp
        src/HexConverter.cpp
        src/PerfUtilities.cpp
        src/ThreadUtilities.cpp
        src/DateTimeUtilities.cpp
        src/Testing.cpp
        include/Arrays.hpp
        include/Random.hpp
)

target_include_directories(utils
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(${PROJECT_NAME}
        main.cpp
)

target_link_libraries(${PROJECT_NAME}
        utils
        pthread
        ${EXTRA_LIBS}
)