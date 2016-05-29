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
#include "ISocketListener.h"

namespace Network {

    class EthernetSocket {
    public:
        // Initialises an Ethernet Socket on the interface provided
        EthernetSocket(const std::string& p_interfaceName);
        virtual ~EthernetSocket();

        std::vector<u_int8_t>& getReceiveBuffer();
        void setReceiveTimeout(uint8_t timeout);
        const MacAddress& getInterfaceMac() const;
        //void setReceiveDataHandler(ISocketListener * iSockListener);

        void send(EthernetFrame& ef, const std::vector<u_int8_t>& data);
        ssize_t receive(const MacAddress * source, const MacAddress * destination, uint16_t type, ISocketListener * iSocketListener);
    private:
        std::string interfaceName;
        int sockfd;
        struct sockaddr_ll socket_address; // Used for index
        MacAddress interfaceMac;
        std::vector<u_int8_t> sendBuffer, receiveBuffer;

        //
        ISocketListener * iSocketListener;
    };

}




#endif //NETWORK_DISCOVERY_ETHERNETSOCKET_H
