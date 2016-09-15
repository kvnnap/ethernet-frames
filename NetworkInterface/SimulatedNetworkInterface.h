//
// Created by kevin on 6/18/16.
//

#ifndef NETWORK_DISCOVERY_SIMULATEDNETWORKINTERFACE_H
#define NETWORK_DISCOVERY_SIMULATEDNETWORKINTERFACE_H

#include <mutex>
#include <atomic>
#include <map>
#include "INetworkInterface.h"
#include "NetworkNode/SwitchNode.h"
#include "NetworkNode/NetDeviceNode.h"

namespace Network {
    class SimulatedNetworkInterface
        : public INetworkInterface
    {

    public:
        // Constructor
        SimulatedNetworkInterface(NetNodePt p_rootNode);

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

        // own methods
        size_t getNumNetDevices() const;
        const std::vector<NetDeviceNode *>& getNetDevices() const;
        const NetNodePt& getNetworkTree() const;

    private:
        NetNodePt rootNode;
        std::vector<NetDeviceNode *> netDevices;
        std::vector<bool> available;

        // Mutex
        std::mutex mtx;
    };
}

#endif //NETWORK_DISCOVERY_SIMULATEDNETWORKINTERFACE_H
