cmake_minimum_required(VERSION 3.14)

project(EzSql LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Test)

add_subdirectory(ezsql)

#
# Tests
#
enable_testing(true)
include(CTest)

add_executable(ezsql_test_stmt test/test_stmt.cpp)
add_test(NAME Test_stmt COMMAND ezsql_test_stmt)
target_include_directories(ezsql_test_stmt PRIVATE ezsql)
target_link_libraries(ezsql_test_stmt PRIVATE EzSql Qt6::Test)

add_executable(ezsql_test_result test/test_result.cpp)
add_test(NAME Test_result COMMAND ezsql_test_result)
target_include_directories(ezsql_test_result PRIVATE ezsql)
target_link_libraries(ezsql_test_result PRIVATE EzSql Qt6::Test)

add_executable(ezsql_test_basedbo test/test_basedbo.cpp)
add_test(NAME Test_basedbo COMMAND ezsql_test_basedbo)
target_include_directories(ezsql_test_basedbo PRIVATE ezsql)
target_link_libraries(ezsql_test_basedbo PRIVATE EzSql Qt6::Test)

add_executable(ezsql_test_db test/test_db.cpp)
add_test(NAME Test_db COMMAND ezsql_test_db)
target_include_directories(ezsql_test_db PRIVATE ezsql)
target_link_libraries(ezsql_test_db PRIVATE EzSql Qt6::Test)
