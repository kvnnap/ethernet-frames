//
// Created by kevin on 6/19/16.
//

#include "NetworkNode.h"
#include "SimulatedNetworkInterface.h"

using namespace std;
using namespace Network;

NetworkNode::~NetworkNode() {

}

NetworkNode::NetworkNode(NetworkNodeType p_type, SimulatedNetworkInterface& p_sim)
    : type (p_type),
      simulatedNetworkInterface (p_sim)
{ }

NetworkNodeType NetworkNode::getType() const {
    return type;
}

void NetworkNode::add(NetNodePt netNodePt) {
    peerNodes.push_back(move(netNodePt));
    peerRefNodes.push_back(this);
}

std::vector<NetNodePt>& NetworkNode::getChildPeerNodes() {
    return peerNodes;
}

std::vector<NetworkNode *>& NetworkNode::getParentPeerNodes() {
    return peerRefNodes;
}

void NetworkNode::send(NetworkNode *to, const uint8_t *buffer) {
    simulatedNetworkInterface.sendQueue(to, buffer);
}














