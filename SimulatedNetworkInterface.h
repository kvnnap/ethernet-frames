//
// Created by kevin on 6/18/16.
//

#ifndef NETWORK_DISCOVERY_SIMULATEDNETWORKINTERFACE_H
#define NETWORK_DISCOVERY_SIMULATEDNETWORKINTERFACE_H

#include <mutex>
#include <atomic>
#include <map>
#include "INetworkInterface.h"
#include "SwitchNode.h"
#include "NetDeviceNode.h"

namespace Network {
    class SimulatedNetworkInterface
        : public INetworkInterface
    {

    public:
        // Constructor
        SimulatedNetworkInterface();

        //
        void sendQueue(NetworkNode* to, const uint8_t * buffer);

        // overridden methods
        int socket(int domain, int type, int protocol) override;
        int ioctl (int fd, unsigned long request, void* args) override;
        int close (int fd) override;
        int setsockopt (int fd, int level, int optname,
                        const void *optval, socklen_t optlen) override;
        ssize_t sendto (int fd, const void *buf, size_t n,
                        int flags, const struct sockaddr* addr,
                        socklen_t addr_len) override;
        ssize_t recvfrom (int fd, void * buf, size_t n,
                          int flags, struct sockaddr* addr,
                          socklen_t * addr_len) override;

    private:
        NetNodePt rootNode;
        std::vector<NetDeviceNode *> netDevices;
        std::vector<bool> available;

        // used only by networknodes
        std::queue<NetworkNode *> nodeSendQueue;

        // Mutex
        std::mutex mtx;
    };
}

#endif //NETWORK_DISCOVERY_SIMULATEDNETWORKINTERFACE_H
