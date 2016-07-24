//
// Created by kevin on 6/19/16.
//

#ifndef NETWORK_DISCOVERY_INETWORKNODE_H
#define NETWORK_DISCOVERY_INETWORKNODE_H

#include <vector>
#include <memory>
#include <queue>
#include "Ethernet/MacAddress.h"
#include "SimulationData.h"

namespace Network {

    enum NetworkNodeType {
        SWITCH,
        DEVICE
    };

    using Port = uint8_t;
    class NetworkNode;
    using NetNodePt = std::unique_ptr<NetworkNode>;

    class SimulatedNetworkInterface;
    class NetworkNode {
    public:

        virtual ~NetworkNode();
        NetworkNode(NetworkNodeType p_type);

        NetworkNodeType getType() const;
        void add(NetNodePt netNodePt);
        std::vector<NetNodePt>& getChildPeerNodes();
        std::vector<NetworkNode *>& getParentPeerNodes();

        static void send(SimulationData& p_simData, std::queue<SimulationData>& nodeSendQueue);

        // virtual methods
        virtual void receive(SimulationData& p_simData, std::queue<SimulationData>& nodeSendQueue) = 0;

    protected:

        // used only by networknodes
        std::queue<SimulationData> nodeSendQueue;

    private:
        const NetworkNodeType type;
        // Owned and need to be cleared on destruction
        std::vector<NetNodePt> peerNodes;
        // Not owned, just references
        std::vector<NetworkNode *> peerRefNodes;
    };



}


#endif //NETWORK_DISCOVERY_INETWORKNODE_H