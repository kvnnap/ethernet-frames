//
// Created by kevin on 5/27/16.
//

#ifndef NETWORK_DISCOVERY_ETHERNETSOCKET_H
#define NETWORK_DISCOVERY_ETHERNETSOCKET_H

#define BUFFER_SIZE 1024

#include <linux/if_packet.h>

#include <cstdint>
#include <string>
#include <vector>

#include "MacAddress.h"
#include "EthernetFrame.h"

namespace Network {

    class EthernetSocket {
    public:
        // Initialises an Ethernet Socket on the interface provided
        EthernetSocket(const std::string& p_interfaceName);
        virtual ~EthernetSocket();

        void send(EthernetFrame& ef, const std::vector<u_int8_t>& data);
        void recv();
    private:
        std::string interfaceName;
        int sockfd;
        struct sockaddr_ll socket_address; // Used for index
        MacAddress interfaceMac;
        std::vector<u_int8_t> sendBuffer, receiveBuffer;
    };

}




#endif //NETWORK_DISCOVERY_ETHERNETSOCKET_H
