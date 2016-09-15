//
// Created by kevin on 6/19/16.
//

#ifndef NETWORK_DISCOVERY_NETDEVICENODE_H
#define NETWORK_DISCOVERY_NETDEVICENODE_H

#include <mutex>
#include <condition_variable>
#include <atomic>
#include "NetworkNode.h"

namespace Network {
    class NetDeviceNode
        : public NetworkNode
    {
    public:
        NetDeviceNode(const MacAddress& p_macAddress);

        const MacAddress& getMacAddress() const;
        void setPromiscuous (bool val);
        bool getPromiscuous() const;
        void setReceiveThreadTimeout(uint32_t p_msTimeout);
        void sendTo(const uint8_t * buffer, size_t lenBuffer);
        ssize_t recvFrom(uint8_t * buffer, size_t lenBuffer);
        // Virtual
        void receive(SimulationData& p_simData, std::queue<SimulationData>& nodeSendQueue) override;
        // Tree Serialiser
        Util::NodePt toTree() const override;

    private:
        MacAddress macAddress;
        std::atomic<uint32_t> msTimeout;
        bool isPromiscuous;

        std::queue<std::vector<uint8_t>> dataPackets;
        std::mutex mtx;
        std::condition_variable cv;
        bool msgsPending;
    };
}


#endif //NETWORK_DISCOVERY_NETDEVICENODE_H
