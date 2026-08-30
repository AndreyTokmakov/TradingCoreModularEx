cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 26)

project(TradingCoreModularEx)
set(PROJECT_VERSION 1.0.0.0)
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
add_compile_options(-c -Wall -O3 -std=c++26)

# 5.15 - My home Ubuntu-20.04
if (CMAKE_SYSTEM_VERSION MATCHES "5.15")
    set(THIRD_PARTY_DIR /home/andtokm/DiskS/ProjectsUbuntu/third_party)
    set(EXTRA_LIBS tbb)
else()
    set(THIRD_PARTY_DIR /home/andrei/Projects/third_party)
endif()

add_subdirectory(trading_core)
add_subdirectory(utilities)
