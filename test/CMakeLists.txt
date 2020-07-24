cmake_minimum_required(VERSION 2.8)

set(CMAKE_MACOSX_RPATH 0)

project(sql-test)

find_package(Qt5 COMPONENTS Core Sql REQUIRED)

set(DEBUG_FLAGS "-std=c++11 -g -O1 -Wall -Wextra -pedantic")
set(RELEASE_FLAGS "-std=c++11 -O3 -Wall -Wextra -pedantic")

set(CMAKE_CXX_FLAGS ${RELEASE_FLAGS})
set(CMAKE_CXX_FLAGS_DEBUG ${DEBUG_FLAGS})
set(CMAKE_CONFIGURATION_TYPES Debug Release)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_directories(sql-test "../")

set(SQL_TEST_SRC test.cpp)
add_executable(sql-test ${SQL_TEST_SRC})

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    Qt5::Core
    Qt5::Sql
)


add_test(all "sql-test")

enable_testing()
