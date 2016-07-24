//
// Created by kevin on 6/19/16.
//

#ifndef NETWORK_DISCOVERY_SWTICHNODE_H
#define NETWORK_DISCOVERY_SWTICHNODE_H

#include <unordered_map>
#include "NetworkNode.h"

namespace std {
    template <>
    struct hash<Network::MacAddress>
    {
        std::size_t operator()(const Network::MacAddress& mac) const
        {
            return mac.getMacArray()[2] << 24 |
                   mac.getMacArray()[3] << 16 |
                   mac.getMacArray()[4] << 8 |
                   mac.getMacArray()[5] << 0;
        }
    };
}

namespace Network {
    class SwitchNode
        : public NetworkNode
    {
    public:
        SwitchNode();

        void receive(SimulationData& p_simData, std::queue<SimulationData>& nodeSendQueue) override;
    private:
        std::unordered_map<MacAddress, NetworkNode *> macPortMap;
    };
}


#endif //NETWORK_DISCOVERY_SWTICHNODE_H
