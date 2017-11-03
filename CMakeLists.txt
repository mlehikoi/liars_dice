cmake_minimum_required(VERSION 3.9)
project (bluff)

find_package(Threads)

add_definitions(
    -g
    -std=c++14
    -Werror
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_definitions(
        -Weverything
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-exit-time-destructors
        -Wno-shadow-uncaptured-local
        -Wno-documentation-unknown-command
    )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set_source_files_properties(test_bluff.cpp PROPERTIES COMPILE_FLAGS -Wno-everything)
endif()

enable_testing()

find_package(Boost COMPONENTS system REQUIRED)

include_directories(SYSTEM
    ../crow/include
    ../rapidjson/include
    ../googletest/googletest/include
    ${Boost_INCLUDE_DIRS}
)

add_library(libdice
    helpers.cpp
    bid.cpp
    dice.cpp
    engine.cpp
    game.cpp
    player.cpp
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

target_link_libraries(runUnitTests
    libdice
    gtest
    gtest_main
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)
add_test(runUnitTests runUnitTests)

find_package(Doxygen)
if (DOXYGEN_FOUND)
    MESSAGE( STATUS "PROJECT_BINARY_DIR: " ${CMAKE_CURRENT_BINARY_DIR} )
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif (DOXYGEN_FOUND)
