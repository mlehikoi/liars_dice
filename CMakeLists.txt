cmake_minimum_required(VERSION 3.9)
project (bluff)

add_definitions(
    -std=c++14
)

find_package(Boost COMPONENTS system REQUIRED)
include_directories(
    ../crow/include
    ${Boost_INCLUDE_DIRS}
)

# clang++ -std=c++14 -x c++-header  -I/usr/local/Cellar/boost/1.65.0/include/ ./crow_all.h -o crow_all.h.pch
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include-pch ../../crow/amalgamate/crow_all.h.pch")

add_executable(bluff bluff.cpp)
target_link_libraries(bluff
    ${Boost_LIBRARIES}
    ${Boost_SYSTEM_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)

if (Tcmalloc_FOUND)
target_link_libraries(example ${Tcmalloc_LIBRARIES})
endif(Tcmalloc_FOUND)
