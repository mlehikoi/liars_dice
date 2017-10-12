cmake_minimum_required(VERSION 3.9)
project (bluff)

add_definitions(
    -std=c++14
)

enable_testing()
find_package(GTest REQUIRED)
MESSAGE(STATUS "GTest" ${GTEST_INCLUDE_DIRS})
MESSAGE(STATUS "GTest" ${GTEST_INCLUDE_DIRS})
find_package(Boost COMPONENTS system REQUIRED)

include_directories(
    ../crow/include
    ../rapidjson/include
    ${Boost_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
)

# clang++ -std=c++14 -x c++-header  -I/usr/local/Cellar/boost/1.65.0/include/ ./crow_all.h -o crow_all.h.pch
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include-pch ../crow_all.h.pch")

add_executable(bluff
    bluff.cpp
    helpers.cpp
    engine.cpp
)

target_link_libraries(bluff
    ${Boost_LIBRARIES}
    ${Boost_SYSTEM_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)

if (GTEST_FOUND)
    add_executable(runUnitTests
        test_bluff.cpp
        engine.cpp
        helpers.cpp)
    target_link_libraries(runUnitTests GTest::GTest GTest::Main ${Boost_LIBRARIES})
    add_test(runUnitTests runUnitTests)
endif(GTEST_FOUND)

if (Tcmalloc_FOUND)
    target_link_libraries(example ${Tcmalloc_LIBRARIES})
endif(Tcmalloc_FOUND)
