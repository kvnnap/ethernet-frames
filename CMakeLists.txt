# CMAKE Version and Project Name
cmake_minimum_required(VERSION 2.8)
project(network_discovery)

# Use C++11, enable all warnings and compile in debug mode
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
#set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_VERBOSE_MAKEFILE ON)

# Add any definitions here

# Find the system thread library and place in CMAKE_THREAD_LIBS_INIT - To link later
find_package (Threads)

# Find include and library paths here

# Global include directories

# network_discovery target
# Find all source files in the below directory automatically
file(GLOB_RECURSE SRC
    Ethernet/*.cpp
    Factory/*.cpp
    Mathematics/*.cpp
    NetworkInterface/*.cpp
    NetworkNode/*.cpp
    Sampler/*.cpp
    Util/*.cpp
    XML/*.cpp
)
add_executable(network_discovery main.cpp ${SRC})
target_include_directories(network_discovery PUBLIC .)
target_link_libraries (network_discovery ${CMAKE_THREAD_LIBS_INIT})