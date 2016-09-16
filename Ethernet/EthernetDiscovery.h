//
// Created by kevin on 5/29/16.
//

#ifndef NETWORK_DISCOVERY_ETHERNETDISCOVERY_H
#define NETWORK_DISCOVERY_ETHERNETDISCOVERY_H

#include <set>

#include "EthernetSocket.h"
#include "ISocketListener.h"
#include "IndexedTopologyTree.h"
#include "Mathematics/Matrix.h"
#include "Util/Node.h"

namespace  Network {

    class EthernetDiscovery
        : public ISocketListener
    {
    public:

        enum MessageType : uint8_t
        {
            EMPTY = 0,
            HELLO = 1,
            BEGIN = 2,
            READY = 3,
            START = 4,
            TEST = 5,
            REQUEST = 6,
            YES = 7,
            NO = 8,
            SEND_PACKET = 9,
            PROBE = 10,
            // Ping Messages
            BEGIN_PING = 11,
            END_PING = 12,
            PING = 13,
            PONG = 14,
            EXIT = 15
        };

        struct PingParameters {
            float stdConfidence;
            float confidenceInterval;
            float measurementNoise;
            float interThresholdCoefficient;
            uint32_t minMeasurements;
            uint32_t maxMeasurements;
        };

        EthernetDiscovery(EthernetSocket& ethernetSocket);

        void setPingParameters(const PingParameters& p_pingParameters);
        void setGroupedSwitches(bool p_groupSwitches);
        Util::NodePt getToplogyTree();
        void slave();
        void clear();

        // Final
        void terminateSlaves();

        // Overrides
        bool dataArrival(EthernetFrame& ef, uint8_t * data, size_t len) override;

    private:

        // Algorithm 1
        void getAllDevices();
        // Algorithm 2
        void partitionBottomLayer();
        // Algorithm 3
        bool testPermutation(const MacAddress &gateway, const MacAddress& i, const MacAddress& j, const MacAddress& k);
        // Algorithm 4
        void discoverNetwork();

        template <class T>
        static std::vector<std::vector<T>> combinations (const std::vector<T>& elems, size_t k);

        // Ping Based Approach - need to still call getAllDevices beforehand
        Mathematics::Matrix<float> startPingBasedDiscovery();
        Mathematics::Matrix<uint32_t> rttToHopCount(const Mathematics::Matrix<float>& rttMatrix) const;
        static std::vector<size_t> hopCountToTopology(const Mathematics::Matrix<uint32_t>& hopMatrix);
        Util::NodePt getTreeFromParentBasedIndexTree(const std::vector<size_t>& parentBasedIndexTree) const;
        Util::NodePt getTreeFromParentBasedIndexTree(const std::vector<size_t>& parentBasedIndexTree, size_t nodeIndex) const;

        // Data
        EthernetSocket & ethernetSocket;
        std::vector<MacAddress> slaveMacs;
        std::vector<std::set<size_t>> connectivitySet;
        IndexedTopologyTree indexedTopologyTree;

        MessageType lastMessage;
        bool testReceived;

        // Ping Based Data
        float pingTime;
        PingParameters pingParameters;
        bool isPingBased;

        // Grouping
        bool groupSwitches;
    };

}

#endif //NETWORK_DISCOVERY_ETHERNETDISCOVERY_H
