cmake_minimum_required (VERSION 2.8.11)
project(final)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

find_package(Boost REQUIRED COMPONENTS system)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(final final_src/main.cpp)
target_link_libraries(final ${Boost_LIBRARIES} pthread)
