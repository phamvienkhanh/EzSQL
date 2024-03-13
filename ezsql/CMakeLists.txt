cmake_minimum_required(VERSION 3.14)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(EzSql SHARED
    ezsql.h
    ezsql.cpp
)

target_include_directories(EzSql
    PUBLIC sqlcipher
)

target_link_directories(EzSql
    PUBLIC sqlcipher/lib
)

target_compile_definitions(EzSql
    PUBLIC "-DSQLITE_ENABLE_PREUPDATE_HOOK"
)

target_link_libraries(EzSql
    PUBLIC Qt6::Core sqlcipher
)
