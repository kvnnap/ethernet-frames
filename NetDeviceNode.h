//
// Created by kevin on 6/19/16.
//

#ifndef NETWORK_DISCOVERY_NETDEVICENODE_H
#define NETWORK_DISCOVERY_NETDEVICENODE_H

#include "NetworkNode.h"

namespace Network {
    class NetDeviceNode
        : public NetworkNode
    {
    public:
        NetDeviceNode(SimulatedNetworkInterface& p_sim, const MacAddress& p_macAddress);

        const MacAddress& getMacAddress() const;
        void setPromiscuous (bool val);
        bool getPromiscuous() const;
        void setReceiveThreadTimeout(uint32_t p_msTimeout);
        void sendTo(const uint8_t * buffer);
        // Virtual
        void receive(NetworkNode* from, const uint8_t * buffer) override;

    private:
        MacAddress macAddress;
        uint32_t msTimeout;
        bool isPromiscuous;
    };
}


#endif //NETWORK_DISCOVERY_NETDEVICENODE_H
