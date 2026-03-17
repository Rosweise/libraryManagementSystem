cmake_minimum_required(VERSION 3.11)
project(lib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

if (POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif ()

FetchContent_Declare(
        sqlite3
        URL https://www.sqlite.org/2024/sqlite-amalgamation-3450200.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_GetProperties(sqlite3)
if (NOT sqlite3_POPULATED)
    FetchContent_Populate(sqlite3)
endif ()

add_executable(${CMAKE_PROJECT_NAME}
        src/main.cpp
        src/database.cpp
        src/sha256.cpp
        src/logger.cpp
        src/logging_functions.cpp
        ${sqlite3_SOURCE_DIR}/sqlite3.c
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${sqlite3_SOURCE_DIR}
)
