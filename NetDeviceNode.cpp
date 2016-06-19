//
// Created by kevin on 6/19/16.
//

#include "NetDeviceNode.h"

using namespace std;
using namespace Network;


NetDeviceNode::NetDeviceNode(SimulatedNetworkInterface& p_sim, const MacAddress &p_macAddress)
    : NetworkNode ( DEVICE, p_sim ),
      macAddress (p_macAddress),
      msTimeout (0),
      isPromiscuous ()
{ }

void NetDeviceNode::receive(NetworkNode *from, const uint8_t *buffer) {
    // Here, we need to wake up waiting thread
}

const MacAddress &NetDeviceNode::getMacAddress() const {
    return macAddress;
}

void NetDeviceNode::setPromiscuous(bool val) {
    isPromiscuous = val;
}

bool NetDeviceNode::getPromiscuous() const {
    return isPromiscuous;
}

void NetDeviceNode::setReceiveThreadTimeout(uint32_t p_msTimeout) {
    msTimeout = p_msTimeout;
}

void NetDeviceNode::sendTo(const uint8_t *buffer) {
    if (getParentPeerNodes().size() > 0) {
        send(getParentPeerNodes()[0], buffer);
    }
}
















