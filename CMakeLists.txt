cmake_minimum_required(VERSION 3.10)
project(lib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(${CMAKE_PROJECT_NAME}
        src/main.cpp
        src/database.cpp
        src/sha256.cpp
        src/logger.cpp
        src/logging_functions.cpp
        lib/sqlite3.c
        lib/sqlite3.h
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
