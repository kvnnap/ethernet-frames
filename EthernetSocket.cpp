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

#include "MacAddress.h"

using namespace std;
using namespace Network;

EthernetSocket::EthernetSocket(const string &p_interfaceName)
    : interfaceName ( p_interfaceName ), socket_address(), sendBuffer (BUFFER_SIZE), receiveBuffer (BUFFER_SIZE)
{
    if (p_interfaceName.length() > IFNAMSIZ) {
        // Throw exception
        throw invalid_argument("Interface name is too long.");
    }

    // Open RAW socket to send on
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
        throw invalid_argument(strerror(errno));
    }

    // Get interface ID
    {
        // value-initialisation will perform zero-initialisation
        struct ifreq if_idx {};
        p_interfaceName.copy(if_idx.ifr_name, p_interfaceName.length());
        if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0) {
            close(sockfd);
            throw invalid_argument(string("SIOCGIFINDEX") + strerror(errno));
        }
        socket_address.sll_ifindex = if_idx.ifr_ifindex;
    }

    // Get interface MAC Address
    {
        // value-initialisation will perform zero-initialisation
        struct ifreq if_mac {};
        p_interfaceName.copy(if_mac.ifr_name, p_interfaceName.length());
        if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0) {
            close(sockfd);
            throw invalid_argument(string("SIOCGIFHWADDR") + strerror(errno));
        }
        interfaceMac.setMacArray(*(u_int8_t(*)[6])&if_mac.ifr_hwaddr.sa_data);
    }

    // Bind to device (For Reading)
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, p_interfaceName.c_str(), IFNAMSIZ - 1) == -1)	{
        close(sockfd);
        throw invalid_argument(string("SO_BINDTODEVICE") + strerror(errno));
    }
}

EthernetSocket::~EthernetSocket() {
    close(sockfd);
}

void EthernetSocket::send(EthernetFrame &ef, const std::vector<u_int8_t> &data) {

    u_int8_t * buff = sendBuffer.data();
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
    if (sendto(sockfd, sendBuffer.data(), size, 0, (struct sockaddr*)&socket_address, sizeof(socket_address)) < 0) {
        throw runtime_error(string("send: ") + strerror(errno));
    }
}


void EthernetSocket::recv() {
    ssize_t numBytes;

    MacAddress broadcastAddress = MacAddress::GetBroadcastMac();

    while((numBytes = recvfrom(sockfd, receiveBuffer.data(), receiveBuffer.size(), 0, NULL, NULL)) != -1) {
        //printf("received: %d bytes\n", numBytes);

        EthernetFrame eff (receiveBuffer.data());

        EthernetFrame * ef = &eff;

        //EthernetFrame * ef = (EthernetFrame *) receiveBuffer.data();

        if (ef->getEtherType() == ETH_P_IP && ef->destinationMac == broadcastAddress) {
            // Process stuff
            cout << "Received Broadcast: Size: " << numBytes << " EF: " << endl << *ef << endl;
        }
    }
}