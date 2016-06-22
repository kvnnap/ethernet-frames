# CMAKE Version and Project Name
cmake_minimum_required(VERSION 2.8)
project(network_discovery)

# Use C++11, enable all warnings and compile in debug mode
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_VERBOSE_MAKEFILE ON)

# Add any definitions here

# Find the system thread library and place in CMAKE_THREAD_LIBS_INIT - To link later
find_package (Threads)

# Find include and library paths here

# Global include directories

# network_discovery target
set(SOURCE_FILES
        main.cpp
        EthernetSocket.cpp EthernetSocket.h
        MacAddress.cpp MacAddress.h
        EthernetFrame.cpp EthernetFrame.h
        ISocketListener.cpp ISocketListener.h
        EthernetDiscovery.cpp EthernetDiscovery.h
        NetworkInterface/INetworkInterface.cpp NetworkInterface/INetworkInterface.h
        NetworkInterface/LinuxNetworkInterface.cpp NetworkInterface/LinuxNetworkInterface.h
        NetworkInterface/SimulatedNetworkInterface.cpp NetworkInterface/SimulatedNetworkInterface.h
        NetworkInterface/NetworkNode.cpp NetworkInterface/NetworkNode.h
        NetworkInterface/SwitchNode.cpp NetworkInterface/SwitchNode.h
        NetworkInterface/NetDeviceNode.cpp NetworkInterface/NetDeviceNode.h
        NetworkInterface/SimulationData.cpp NetworkInterface/SimulationData.h)
add_executable(network_discovery ${SOURCE_FILES})
target_link_libraries (network_discovery ${CMAKE_THREAD_LIBS_INIT})