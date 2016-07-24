//
// Created by kevin on 6/19/16.
//

#include "NetDeviceNode.h"

using namespace std;
using namespace Network;


NetDeviceNode::NetDeviceNode(const MacAddress &p_macAddress)
    : NetworkNode ( DEVICE ),
      macAddress (p_macAddress),
      msTimeout (0),
      isPromiscuous (),
      msgsPending ()
{ }

void NetDeviceNode::receive(SimulationData& p_simData, queue<SimulationData>& nodeSendQueue) {
    {
        lock_guard<mutex> lock(mtx);
        dataPackets.push(std::vector<uint8_t>(p_simData.buffer, p_simData.buffer + p_simData.lenBuffer));
        // Here, we need to wake up waiting thread
        msgsPending = true;
    }
    cv.notify_one();
}

ssize_t NetDeviceNode::recvFrom(uint8_t *buffer, size_t lenBuffer) {
    unique_lock<mutex> lock (mtx);
    // process the messages that we can find
    if (dataPackets.empty()) {
        // We need to wait
        msgsPending = false;
        if (msTimeout == 0) {
            cv.wait(lock, [this]{return msgsPending;});
        } else {
            if(!cv.wait_for(lock, chrono::milliseconds(msTimeout), [this]{return msgsPending;})) {
                errno = EWOULDBLOCK;
                return -1;
            }
        }

    }

    size_t  ret (0);
    // We have data, return it
    if (dataPackets.front().size() <= lenBuffer) {
        copy(dataPackets.front().begin(), dataPackets.front().end(), buffer);
        ret = dataPackets.front().size();
        dataPackets.pop();
    } else {
        copy(dataPackets.front().begin(), dataPackets.front().begin() + lenBuffer, buffer);
        // remove first lenBuffer data
        dataPackets.front().erase(dataPackets.front().begin(), dataPackets.front().begin() + lenBuffer);
        ret = lenBuffer;
    }

    return ret;
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

void NetDeviceNode::sendTo(const uint8_t *buffer, size_t lenBuffer) {
    queue<SimulationData> nodeSendQueue;

    if (getParentPeerNodes().size() > 0) {
        SimulationData simData;
        simData.from = this;
        simData.to = getParentPeerNodes()[0];
        simData.buffer = buffer;
        simData.lenBuffer = lenBuffer;
        send(simData, nodeSendQueue);
    }

    while(!nodeSendQueue.empty()) {
        SimulationData & simData = nodeSendQueue.front();
        simData.to->receive(simData, nodeSendQueue);
        nodeSendQueue.pop();
    }
}




















