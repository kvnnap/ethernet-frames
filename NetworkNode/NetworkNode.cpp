//
// Created by kevin on 6/19/16.
//

#include "NetworkNode.h"
#include "NetworkInterface/SimulatedNetworkInterface.h"

using namespace std;
using namespace Network;

NetworkNode::~NetworkNode() {

}

NetworkNode::NetworkNode(NetworkNodeType p_type)
    : type (p_type)
{ }

NetworkNodeType NetworkNode::getType() const {
    return type;
}

void NetworkNode::add(NetNodePt netNodePt) {
    netNodePt->peerRefNodes.push_back(this);
    peerNodes.push_back(move(netNodePt));
}

std::vector<NetNodePt>& NetworkNode::getChildPeerNodes() {
    return peerNodes;
}

std::vector<NetworkNode *>& NetworkNode::getParentPeerNodes() {
    return peerRefNodes;
}

void NetworkNode::send(SimulationData& p_simData, queue<SimulationData>& nodeSendQueue) {
    nodeSendQueue.push(p_simData);
}

const vector<NetNodePt> &NetworkNode::getChildPeerNodes() const {
    return peerNodes;
}














