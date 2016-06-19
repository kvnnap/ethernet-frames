//
// Created by kevin on 6/19/16.
//

#include "SwitchNode.h"
#include "EthernetFrame.h"

using namespace std;
using namespace Network;

SwitchNode::SwitchNode(SimulatedNetworkInterface& p_sim)
    : NetworkNode (SWITCH, p_sim)
{ }

void SwitchNode::receive(NetworkNode *from, const uint8_t *buffer) {
    // Extract EthernetFrame
    EthernetFrame ef (buffer);
    if (macPortMap.find(ef.sourceMac) == macPortMap.end()) {
        // not found - simply add it
        macPortMap.insert(make_pair(ef.sourceMac, from));
    }

    // Check if we know to whom to send this
    // We don't
    if (macPortMap.find(ef.destinationMac) == macPortMap.end()) {
        // Send to everyone except from source
        for (NetNodePt& child : getChildPeerNodes()) {
            if (from != child.get()) {
                send(child.get(), buffer);
            }
        }
        for (NetworkNode * node : getParentPeerNodes()) {
            if (from != node) {
                send(node, buffer);
            }
        }
    } else {
        NetworkNode * node = macPortMap[ef.destinationMac]; //macPortMap.find(ef.destinationMac)->second;
        if (from != node) {
            send(node, buffer);
        }
    }
}
