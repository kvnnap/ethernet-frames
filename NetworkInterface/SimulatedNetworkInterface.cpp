//
// Created by kevin on 6/18/16.
//

#include <iostream>
#include <queue>
#include "SimulatedNetworkInterface.h"
#include "NetDeviceNode.h"

using namespace std;
using namespace Network;

SimulatedNetworkInterface::SimulatedNetworkInterface()
{
    // Generate dummy network
    rootNode.reset(new SwitchNode(*this));

    // Connect rp3 and rp2 to Main Switch
    MacAddress macAddress;
    macAddress.setArrayElement(5, 1);
    rootNode->add(NetNodePt(new NetDeviceNode(*this, macAddress)));
    macAddress.setArrayElement(5, 2);
    rootNode->add(NetNodePt(new NetDeviceNode(*this, macAddress)));

    // Connect Main Switch to second switch
    SwitchNode * s2;
    rootNode->add(NetNodePt(s2 = new SwitchNode(*this)));
    // Connect rp3 and rp2 to Second Switch
    macAddress.setArrayElement(5, 3);
    s2->add(NetNodePt(new NetDeviceNode(*this, macAddress)));
    macAddress.setArrayElement(5, 4);
    s2->add(NetNodePt(new NetDeviceNode(*this, macAddress)));


    // Assume we know nothing of this network
    // Scan and save a reference to NetDevices
    std::queue<NetworkNode *> switchQueue;
    switchQueue.push(rootNode.get());
    while (!switchQueue.empty()) {
        NetworkNode * node = switchQueue.front();
        switchQueue.pop();
        for (NetNodePt& child : node->getChildPeerNodes()) {
            if (child->getType() == NetworkNodeType::DEVICE) {
                netDevices.push_back(static_cast<NetDeviceNode *>(child.get()));
            } else if (child->getType() == NetworkNodeType::SWITCH) {
                switchQueue.push(child.get());
            }
        }
    }

    available.resize(netDevices.size(), true);
    //cout << "NetDeviceCount: " << netDevices.size() << endl;
}

// Request new socket file-descriptor
int SimulatedNetworkInterface::socket(int domain, int type, int protocol) {
    lock_guard<mutex> lock (mtx);

    for (size_t i = 0; i < available.size(); ++i) {
        if (available[i]) {
            available[i] = false;
            return static_cast<int>(i);
        }
    }

    return -1;
}

int SimulatedNetworkInterface::ioctl(int fd, unsigned long request, void *args) {
    lock_guard<mutex> lock (mtx);
    if (fd < 0 || static_cast<size_t>(fd) >= available.size() || available[fd]) {
        return -1;
    }
    switch (request) {

        // Get interface index
        case SIOCGIFINDEX:
            break;

        // Get MAC
        case SIOCGIFHWADDR:
        {
            struct ifreq * if_mac = static_cast<struct ifreq*>(args);
            netDevices[fd]->getMacAddress().copyTo((uint8_t *)if_mac->ifr_hwaddr.sa_data);
        }
            break;

        case SIOCGIFFLAGS:
        {
            struct ifreq *if_opts = static_cast<struct ifreq *>(args);
            if_opts->ifr_flags = netDevices[fd]->getPromiscuous() ? IFF_PROMISC : 0;
        }
            break;

        case SIOCSIFFLAGS:
        {
            struct ifreq *if_opts = static_cast<struct ifreq *>(args);
            netDevices[fd]->setPromiscuous((if_opts->ifr_flags & IFF_PROMISC) > 0);
        }
            break;

        default:
            return -1;
    }
    return 0;
}

int SimulatedNetworkInterface::close(int fd) {
    lock_guard<mutex> lock (mtx);
    if (fd < 0 || static_cast<size_t>(fd) >= available.size() || available[fd]) {
        return -1;
    }
    available[fd] = true;
    netDevices[fd]->setPromiscuous(false);
    netDevices[fd]->setReceiveThreadTimeout(0);
    return 0;
}

int SimulatedNetworkInterface::setsockopt(int fd, int level, int optname, const void *optval,
                                                   socklen_t optlen) {
    lock_guard<mutex> lock (mtx);
    if (fd < 0 || static_cast<size_t>(fd) >= available.size() || available[fd]) {
        return -1;
    }
    switch (optname) {
        case SO_BINDTODEVICE:
            break;
        case SO_RCVTIMEO:
        {
            struct timeval *tv = (struct timeval *) (optval);
            uint32_t msTimeout = static_cast<uint32_t>(tv->tv_sec * 1000 + tv->tv_usec / 1000);
            if ((tv->tv_usec % 1000) > 0) {
                ++msTimeout;
            }
            netDevices[fd]->setReceiveThreadTimeout(msTimeout);
        }
            break;
        default:
            return -1;
    }

    return 0;
}

ssize_t SimulatedNetworkInterface::sendto(int fd, const void *buf, size_t n, int flags,
                                                   const struct sockaddr *addr, socklen_t addr_len) {
    lock_guard<mutex> lock (mtx);
    if (fd < 0 || static_cast<size_t>(fd) >= available.size() || available[fd]) {
        return -1;
    }
    const uint8_t * const buffer = static_cast<const uint8_t *>(buf);
    netDevices[fd]->sendTo(buffer, n);

    while(!nodeSendQueue.empty()) {
        SimulationData & simData = nodeSendQueue.front();
        simData.to->receive(simData);
        nodeSendQueue.pop();
    }

    return 0;
}

ssize_t SimulatedNetworkInterface::recvfrom(int fd, void *buf, size_t n, int flags, struct sockaddr *addr,
                                                     socklen_t *addr_len) {
    uint8_t * buffer;
    NetDeviceNode * netNode;

    {
        lock_guard<mutex> lock(mtx);

        if (fd < 0 || static_cast<size_t>(fd) >= available.size() || available[fd]) {
            return -1;
        }
        buffer = static_cast<uint8_t *>(buf);
        netNode = netDevices[fd];
    }
    return netNode->recvFrom(buffer, n);
}

void SimulatedNetworkInterface::sendQueue(SimulationData& p_simData) {
    //lock_guard<mutex> lock (mtx);
    nodeSendQueue.push(p_simData);
}
















