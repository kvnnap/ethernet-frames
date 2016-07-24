//
// Created by kevin on 5/27/16.
//

#include "EthernetSocket.h"

#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <string>

#include <iostream>
#include <stdexcept>


using namespace std;
using namespace Network;

EthernetSocket::EthernetSocket(const string &p_interfaceName, INetworkInterface& p_netInterface)
    : interfaceName ( p_interfaceName ), socket_address(), sendBuffer (BUFFER_SIZE), receiveBuffer (BUFFER_SIZE),
      netInterface ( p_netInterface )
{
    if (p_interfaceName.length() > IFNAMSIZ) {
        // Throw exception
        throw invalid_argument("INetworkInterface name is too long.");
    }

    // Open RAW socket to send on
    if ((sockfd = netInterface.socket(AF_PACKET, SOCK_RAW, htons(CUSTOM_ETH_TYPE))) == -1) {
        throw invalid_argument(strerror(errno));
    }

    // Get interface ID
    {
        // value-initialisation will perform zero-initialisation
        struct ifreq if_idx {};
        p_interfaceName.copy(if_idx.ifr_name, p_interfaceName.length());
        if (netInterface.ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0) {
            netInterface.close(sockfd);
            throw invalid_argument(string("SIOCGIFINDEX") + strerror(errno));
        }
        socket_address.sll_ifindex = if_idx.ifr_ifindex;
    }

    // Get interface MAC Address
    {
        // value-initialisation will perform zero-initialisation
        struct ifreq if_mac {};
        p_interfaceName.copy(if_mac.ifr_name, p_interfaceName.length());
        if (netInterface.ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0) {
            netInterface.close(sockfd);
            throw invalid_argument(string("SIOCGIFHWADDR") + strerror(errno));
        }
        interfaceMac.setMacArray(*(uint8_t(*)[6])&if_mac.ifr_hwaddr.sa_data);
    }

    // Set interface to promiscuous mode - Without this, only broadcasts and packets directed to us are received ( and sent?)
    {
        struct ifreq if_opts {};
        p_interfaceName.copy(if_opts.ifr_name, p_interfaceName.length());
        if (netInterface.ioctl(sockfd, SIOCGIFFLAGS, &if_opts) < 0) {
            netInterface.close(sockfd);
            throw invalid_argument(string("SIOCGIFFLAGS") + strerror(errno));
        }
        // Add promiscuous mode to flags
        if_opts.ifr_flags |= IFF_PROMISC;
        if (netInterface.ioctl(sockfd, SIOCSIFFLAGS, &if_opts) < 0) {
            netInterface.close(sockfd);
            throw invalid_argument(string("SIOCSIFFLAGS") + strerror(errno));
        }
    }

    // Bind to device (For Reading)
    if (netInterface.setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, p_interfaceName.c_str(), IFNAMSIZ - 1) == -1)	{
        netInterface.close(sockfd);
        throw invalid_argument(string("SO_BINDTODEVICE") + strerror(errno));
    }
}

EthernetSocket::~EthernetSocket() {
    netInterface.close(sockfd);
}

void EthernetSocket::send(EthernetFrame &ef, const DataBuffer &data) {

    uint8_t * buff = sendBuffer.data();
    size_t size {};

    if (ef.sourceMac.isUnset()) {
        ef.sourceMac = interfaceMac;
    }

    // Copy Supplied Ethernet Frame
    ef.copyTo(buff);
    size += ef.destinationMac.getMacArray().size() + ef.sourceMac.getMacArray().size() + sizeof(u_int16_t);
    buff += size;

    // Copy payload data
    if (sendBuffer.size() - size < data.size()) {
        throw runtime_error("send: Data too large");
    }
    copy(data.begin(), data.end(), buff);
    size += data.size();

    // Send packet
    if (netInterface.sendto(sockfd, sendBuffer.data(), size, 0, (struct sockaddr*)&socket_address, sizeof(socket_address)) < 0) {
        throw runtime_error(string("send: ") + strerror(errno));
    }
}

ssize_t EthernetSocket::receive(ISocketListener * iSocketListener, const MacAddress * destination, const MacAddress * source) {
    ssize_t numBytes;//, validMsgBytes = 0;

    while((numBytes = netInterface.recvfrom(sockfd, receiveBuffer.data(), receiveBuffer.size(), 0, NULL, NULL)) != -1) {
        // We need to make sure we don't break strict type-aliasing rules
        // Check whether G++ guarantees type-punning using union
        // The standard does not allow it for sure
        //EthernetFrame * ef = (EthernetFrame *) receiveBuffer.data();

        EthernetFrame eff (receiveBuffer.data());

        EthernetFrame * ef = &eff;

        if (
                   (ef->getEtherType() != CUSTOM_ETH_TYPE)
                || (destination && ef->destinationMac != *destination)
                || (source && ef->sourceMac != *source)
            )
        {
            continue;
        }

        // Process stuff
        if (iSocketListener->dataArrival(*ef, (uint8_t*)(receiveBuffer.data() + sizeof(*ef)), numBytes - sizeof(*ef))) {
            continue;
        }

        return numBytes;
    }

    // Error stage

    // exit due to timeout
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return -numBytes;
    }

    throw runtime_error(string("read error: ") + strerror(errno));
}

const DataBuffer &EthernetSocket::getReceiveBuffer() const {
    return receiveBuffer;
}

void EthernetSocket::setReceiveTimeout(uint16_t timeout) {
    // set timeout
    struct timeval tv { timeout / 1000 , (timeout % 1000) * 1000 };
    netInterface.setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));
}

const MacAddress &EthernetSocket::getInterfaceMac() const {
    return interfaceMac;
}

/*void EthernetSocket::setReceiveDataHandler(ISocketListener *iSockListener) {
    iSocketListener = iSockListener;
}*/





