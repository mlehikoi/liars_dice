cmake_minimum_required(VERSION 3.9)
project (bluff)

add_definitions(
    -g
    -std=c++14
)

enable_testing()

find_package(Boost COMPONENTS system REQUIRED)

include_directories(
    ../crow/include
    ../rapidjson/include
    ${Boost_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
)

add_library(libdice
    helpers.cpp
    bid.cpp
    engine.cpp
    game.cpp
)
add_executable(bluff
    bluff.cpp
)

target_link_libraries(bluff
    libdice
    ${Boost_LIBRARIES}
    ${Boost_SYSTEM_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    link_directories(../googletest/build-clang/googlemock/gtest)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    link_directories(../googletest/build-gcc/googlemock/gtest)
endif()
add_executable(runUnitTests
    test_bluff.cpp
)

target_link_libraries(runUnitTests libdice gtest gtest_main ${Boost_LIBRARIES})
add_test(runUnitTests runUnitTests)
