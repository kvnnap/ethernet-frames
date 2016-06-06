//
// Created by kevin on 5/27/16.
//

#ifndef NETWORK_DISCOVERY_ETHERNETSOCKET_H
#define NETWORK_DISCOVERY_ETHERNETSOCKET_H

#define BUFFER_SIZE 1024
#define CUSTOM_ETH_TYPE ETH_P_802_EX1

#include <linux/if_packet.h>

#include <cstdint>
#include <string>
#include <vector>

#include "MacAddress.h"
#include "EthernetFrame.h"
#include "ISocketListener.h"

namespace Network {

    using DataBuffer = std::vector<uint8_t>;

    class EthernetSocket {
    public:
        // Initialises an Ethernet Socket on the interface provided
        EthernetSocket(const std::string& p_interfaceName);
        ~EthernetSocket();

        const DataBuffer& getReceiveBuffer() const;
        void setReceiveTimeout(uint16_t timeout);
        const MacAddress& getInterfaceMac() const;
        //void setReceiveDataHandler(ISocketListener * iSockListener);

        void send(EthernetFrame& ef, const DataBuffer& data);
        ssize_t receive(ISocketListener * iSocketListener, const MacAddress * destination = nullptr, const MacAddress * source = nullptr);
    private:
        std::string interfaceName;
        int sockfd;
        struct sockaddr_ll socket_address; // Used for index
        MacAddress interfaceMac;
        DataBuffer sendBuffer, receiveBuffer;

        //
        ISocketListener * iSocketListener;
    };

}




#endif //NETWORK_DISCOVERY_ETHERNETSOCKET_H
