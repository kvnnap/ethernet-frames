cmake_minimum_required(VERSION 2.8)
project(network_discovery)

#set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

set(SOURCE_FILES
        main.cpp
        EthernetSocket.cpp EthernetSocket.h
        MacAddress.cpp MacAddress.h
        EthernetFrame.cpp EthernetFrame.h
        ISocketListener.cpp ISocketListener.h
        EthernetDiscovery.cpp EthernetDiscovery.h
        INetworkInterface.cpp INetworkInterface.h
        LinuxNetworkInterface.cpp LinuxNetworkInterface.h
        SimulatedNetworkInterface.cpp SimulatedNetworkInterface.h)

add_executable(network_discovery ${SOURCE_FILES})