cmake_minimum_required(VERSION 3.9)
project (bluff)

find_package(Threads)

add_definitions(
    -g
    # -O2
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
        # For plantuml
        -Wno-documentation-unknown-command
        # virtual methods in header files
        -Wno-weak-vtables
    )
endif()

enable_testing()

find_package(Boost COMPONENTS system REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(SYSTEM
    ../crow/include
    ../rapidjson/include
    ../googletest/googletest/include
    ../brotli/c/include
    ${Boost_INCLUDE_DIRS}
)

add_library(libdice
    bid.cpp
    brotli.cpp
    dice.cpp
    engine.cpp
    expires.cpp
    filehelpers.cpp
    game.cpp
    helpers.cpp
    player.cpp
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    link_directories(../brotli/build-clang)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    link_directories(../brotli/build-gcc)
endif()

add_executable(bluff
    bluff.cpp
)

target_link_libraries(bluff
    libdice
    brotlicommon-static
    brotlienc-static
    brotlidec-static
    ${Boost_LIBRARIES}
    ${Boost_SYSTEM_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)

set(TEST_FILES
    test_bluff.cpp
    test/test_dice.cpp
    test/test_game.cpp
    test/test_player.cpp
    test/test_brotli.cpp
    test/test_httphelpers.cpp
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    link_directories(../googletest/build-clang/googlemock/gtest)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    link_directories(../googletest/build-gcc/googlemock/gtest)
endif()

add_executable(runUnitTests ${TEST_FILES})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set_source_files_properties(
        ${TEST_FILES}
        PROPERTIES COMPILE_FLAGS -Wno-everything)
endif()

target_link_libraries(runUnitTests
    libdice
    gtest
    gtest_main
    brotlicommon-static
    brotlienc-static
    brotlidec-static
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
