cmake_minimum_required (VERSION 2.8.11)
project(final)

set (CMAKE_CXX_STANDARD 11)
find_package(Boost REQUIRED COMPONENTS system)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(final final/main.cpp)
target_link_libraries(final ${Boost_LIBRARIES} pthread)
