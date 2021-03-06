//
// Created by kevin on 5/29/16.
//

#include <sstream>
#include <numeric>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <chrono>
#include "EthernetDiscovery.h"
#include "Mathematics/Statistics.h"
#include "Mathematics/SetOperations.h"
#include "DataEncoder.h"

using namespace Network;
using namespace Mathematics;
using namespace std;

EthernetDiscovery::EthernetDiscovery(EthernetSocket &ethernetSocket)
        : ethernetSocket ( ethernetSocket ),
          lastMessage (EMPTY),
          testReceived ( false ),
          pingTime ( 0.f ),
          isPingBased ( false ),
          groupSwitches (false)

{ }


void EthernetDiscovery::clear() {
    slaveMacs.clear();
    connectivitySet.clear();
    indexedTopologyTree.clear();
    lastMessage = EMPTY;
    testReceived = false;
    pingTime = 0.f;
    isPingBased = false;
    groupSwitches = false;
}

// returning false breaks the dataArrival infinite loop
// or set a timeout interval..
bool EthernetDiscovery::dataArrival(Network::EthernetFrame &ef, uint8_t *data, size_t len) {

    lastMessage = static_cast<MessageType>(*data);

    switch (*data) {

        // Both Master and Slave Message
        case MessageType::HELLO:
        {
            if (ef.destinationMac == MacAddress::BroadcastMac) {
                // Just received a broadcast HELLO, Reply appropriately
                EthernetFrame replyEf;
                replyEf.destinationMac = ef.sourceMac;
                replyEf.sourceMac = ethernetSocket.getInterfaceMac();
                ethernetSocket.send(replyEf, {MessageType::HELLO});
            } else {
                // Add to list - Verify that we are getToplogyTree
                slaveMacs.push_back(ef.sourceMac);
            }
        }
            break;

        // Slave Message - Broadcasts from First Mac (UA1) and teaches
        // Second Mac (UA2) to the closest switch (switch attached to us)
        // Parameters are UA1 and UA2
        case MessageType::BEGIN:
        {
            // Act only if this packet was meant for this slave - switch might still be learning
            if (ef.destinationMac == ethernetSocket.getInterfaceMac()) {

                // 2) Send an empty frame with source in param (UA1)
                // And destination is broadcast
                EthernetFrame replyEf;
                replyEf.destinationMac = MacAddress::BroadcastMac;
                replyEf.sourceMac.copyFrom(data + 2); // UA1
                ethernetSocket.send(replyEf, {MessageType::EMPTY});

                // 3) Let ONLY the closest switch learn about UA2
                replyEf.destinationMac = replyEf.sourceMac; // UA1
                replyEf.sourceMac.copyFrom(data + 8); // UA2
                ethernetSocket.send(replyEf, {MessageType::EMPTY});

                // 4) Send Ready to Master
                replyEf.destinationMac = ef.sourceMac;
                replyEf.sourceMac = ethernetSocket.getInterfaceMac();
                ethernetSocket.send(replyEf, {MessageType::READY});

                // Reset test every time we Begin
                testReceived = false;
            }
        }
            break;

        // Master Message
        case MessageType::READY:
        {
            // Act only if this packet was meant for this Master
            if (ef.destinationMac == ethernetSocket.getInterfaceMac()) {
                return false;
            }
        }
            break;

        // Slave Message - First Mac to send request to
        case MessageType::START:
        {
            // Act only if this packet was meant for this Master
            if (ef.destinationMac == ethernetSocket.getInterfaceMac()) {
                // 11) Send TEST to UA2
                EthernetFrame replyEf;
                replyEf.destinationMac.copyFrom(data + 8); // UA2
                replyEf.sourceMac = ethernetSocket.getInterfaceMac();
                ethernetSocket.send(replyEf, {MessageType::TEST});

                // 12) Send Request
                replyEf.destinationMac.copyFrom(data + 2); // slaveMacJ
                DataBuffer buff(8);
                buff[0] = REQUEST;
                buff[1] = sizeof(MacAddress);
                ef.sourceMac.copyTo(buff.data() + 2);
                ethernetSocket.send(replyEf, buff);
            }
        }
            break;

        // Slave Message
        case MessageType::TEST:
        {
            testReceived = true;
        }
            break;

        // Slave Message
        case MessageType::REQUEST:
        {
            // Act only if this packet was meant for this Slave
            if (ef.destinationMac == ethernetSocket.getInterfaceMac()) {
                // 13) Respond with yes or no
                EthernetFrame replyEf;
                replyEf.destinationMac.copyFrom(data + 2); // Master MAC
                replyEf.sourceMac = ethernetSocket.getInterfaceMac();
                ethernetSocket.send(replyEf, {testReceived ? MessageType::YES : MessageType::NO});
            }
        }
            break;

        // Host Message
        case MessageType::YES:
        {
            if (ef.destinationMac == ethernetSocket.getInterfaceMac()) {
                return false;
            }
        }
            break;

        // Host Message
        case MessageType::NO:
        {
            if (ef.destinationMac == ethernetSocket.getInterfaceMac()) {
                return false;
            }
        }
            break;

        // Slave Message
        case MessageType::SEND_PACKET:
        {
            // Act only if this packet was meant for this Slave
            // Param 1 = Source (6 bytes) 2 - 7
            // Param 2 = Destination (6 bytes) 8 - 13
            // Param 3 = Variable Size (another Message)
            if (ef.destinationMac == ethernetSocket.getInterfaceMac()) {
                EthernetFrame replyEf;
                replyEf.destinationMac.copyFrom(data + 8);
                replyEf.sourceMac.copyFrom(data + 2);
                DataBuffer buff (data[1] - (uint8_t)12);
                data += 14;
                copy(data, data + buff.size(), buff.data());
                ethernetSocket.send(replyEf, buff);

                // Send Ready to Master
                replyEf.destinationMac = ef.sourceMac;
                replyEf.sourceMac = ethernetSocket.getInterfaceMac();
                ethernetSocket.send(replyEf, {MessageType::READY});
            }
        }
            break;

        // Slave Message
        case MessageType::PROBE:
        {
            // The destination will be different this time
            // Param 1 = Mac to reply to (6 bytes) 2 - 7
            EthernetFrame replyEf;
            replyEf.destinationMac.copyFrom(data + 2);
            replyEf.sourceMac = ethernetSocket.getInterfaceMac();
            ethernetSocket.send(replyEf, {MessageType::YES});
        }
            break;

        // Ping Based Approach
        // Slave Message
        case MessageType::BEGIN_PING:
        {
            // Setup payload to send
            DataBuffer payload (1000);
            payload[0] = PING;
            payload[1] = 255; // hmm.. need 2 bytes for size
            iota(payload.begin() + 2, payload.end(), 0);

            // Setup message
            EthernetFrame macToPingEf;
            macToPingEf.destinationMac.copyFrom(data + 2);

            // Parameters
            float stdConfidence = DataEncoder::readValFromNetworkBytes<float>(data + 8);
            float confidenceIntervalThreshold = DataEncoder::readValFromNetworkBytes<float>(data + 8 + sizeof(float));
            size_t minMeasurements = DataEncoder::readValFromNetworkBytes<uint32_t>(data + 8 + 2 * sizeof(float));
            size_t maxMeasurements = DataEncoder::readValFromNetworkBytes<uint32_t>(data + 8 + 2 * sizeof(float) + sizeof(uint32_t));

            // Rtt times
            vector<float> rttTimes;
            bool confident = false;

            ethernetSocket.setReceiveTimeout(1000);

            do {
                using namespace chrono;

                // Setup timer
                high_resolution_clock::time_point t1 = high_resolution_clock::now();

                // Send
                ethernetSocket.send(macToPingEf, payload);

                // Receive - This is recursive - Data must be for us and coming from the recepient
                if (ethernetSocket.receive(this, &ethernetSocket.getInterfaceMac(), &macToPingEf.destinationMac) < 0) {
                    // Timeout - reset timer
                    continue;
                }

                // Sanity check
                if (lastMessage != PONG) {
                    throw runtime_error("Slave: Expected Pong but got: " + to_string(static_cast<uint32_t>(lastMessage)));
                }

                //
                high_resolution_clock::time_point t2 = high_resolution_clock::now();

                // calculate duration
                duration<float, milli> fMilliSec = t2 - t1;

                // Push data
                rttTimes.push_back(fMilliSec.count());

                if (rttTimes.size() == minMeasurements) {
                    // Do twice more measurements if not confident enough
                    minMeasurements *= 2;

                    // Check if we're confident enough this time round
                    const float sampleStdDeviation = Statistics::SampleStdDeviation(rttTimes);
                    const float stdDeviationOfTheMean = Statistics::StdDeviationOfTheMean(sampleStdDeviation, rttTimes.size());

                    float confidenceInterval = stdDeviationOfTheMean * stdConfidence;

                    confident = confidenceInterval <= confidenceIntervalThreshold;
                }

            } while (!confident && rttTimes.size() < maxMeasurements);

            ethernetSocket.setReceiveTimeout(0);

            // Our mean value has the required confidence or we reached the limit
            /*cout << "Total Measurements: " << rttTimes.size() << " stdConfidence: " << stdConfidence <<
                    " confidenceIntervalThreshold: " << confidenceIntervalThreshold << endl;*/
            EthernetFrame replyEf;
            replyEf.destinationMac = ef.sourceMac;

            DataBuffer buff (2 + sizeof(float));
            buff[0] = MessageType::END_PING;
            buff[1] = sizeof(float);
            DataEncoder::writeValToNetworkBytes(Statistics::Mean(rttTimes), buff.data() + 2);
            ethernetSocket.send(replyEf, buff);
        }
            break;

        // Slave Message - SPECIAL SLAVE MESSAGE for recursive call
        case MessageType::PONG:
        {
            if (ef.destinationMac == ethernetSocket.getInterfaceMac()) {
                return false;
            }
        }
            break;

        // Slave Message
        case MessageType::PING:
        {
            // Reply with Pong
            EthernetFrame replyEf;
            replyEf.destinationMac = ef.sourceMac;
            ethernetSocket.send(replyEf, {MessageType::PONG});
        }
            break;

        // Host Message
        case MessageType::END_PING:
        {
            pingTime = DataEncoder::readValFromNetworkBytes<float>(data + 2);
            if (ef.destinationMac == ethernetSocket.getInterfaceMac()) {
                return false;
            }
        }
            break;

        // Slave Message - Message meant to terminate the processes
        case MessageType::EXIT:
        {
            return false;
        }
            break;

        case MessageType::EMPTY:
        default:
            return true;
    }

    return true;
}

// Algorithm I
void EthernetDiscovery::getAllDevices() {
    slaveMacs.clear();

    // Send a broadcast packet to everyone
    EthernetFrame ef;
    ef.destinationMac = MacAddress::BroadcastMac;
    ethernetSocket.send(ef, {MessageType::HELLO});

    // Receive the messages with destination Mac equivalent to ours
    ethernetSocket.setReceiveTimeout(500);
    ethernetSocket.receive(this, &ethernetSocket.getInterfaceMac());
    ethernetSocket.setReceiveTimeout(0);

    // Sort macs in ascending order - only for visual analysis purposes
    sort(slaveMacs.begin(), slaveMacs.end());
}

void EthernetDiscovery::partitionBottomLayer() {

    connectivitySet.clear();

    // Build Connectivity Set
    for (size_t i = 0; i < slaveMacs.size(); ++i) {
        connectivitySet.push_back({i});
    }

    if (!groupSwitches) {
        return;
    }

    EthernetFrame ef;
    DataBuffer buffBegin (14);
    buffBegin[0] = BEGIN;
    buffBegin[1] = sizeof(MacAddress) * 2;
    MacAddress::UA1.copyTo(buffBegin.data() + 2);
    MacAddress::UA2.copyTo(buffBegin.data() + 8);

    DataBuffer buffStart (14);
    buffStart[0] = START;
    buffStart[1] = sizeof(MacAddress) * 2;
    // Mac to test with
    MacAddress::UA2.copyTo(buffStart.data() + 8);

    for(size_t i = 0; i < connectivitySet.size(); ++i) {

        const MacAddress& slaveMacI = slaveMacs[*connectivitySet[i].begin()];

        // 1) Send Begin UA1
        ef.destinationMac = slaveMacI;
        ethernetSocket.send(ef, buffBegin);

        // 2-3) executed by slave
        // Wait for READY message from Slave
        //ready = false;
        ethernetSocket.receive(this, &ethernetSocket.getInterfaceMac(), &slaveMacI);

        if (lastMessage != READY) {
            throw runtime_error("Algorithm 2 Step 4 (out of sync) - Last message is not READY");
        }

        for(size_t j = i + 1; j < connectivitySet.size(); ++j) {

            const MacAddress& slaveMacJ = slaveMacs[*connectivitySet[j].begin()];

            // 6) Send Begin
            ef.destinationMac = slaveMacJ;
            ethernetSocket.send(ef, buffBegin);

            // 7-8) done by slave
            ethernetSocket.receive(this, &ethernetSocket.getInterfaceMac(), &slaveMacJ);

            // 10) Master sends Start with slaveMacJ to slaveMacI

            // Build packet
            ef.destinationMac = slaveMacI;

            // Mac to send request to
            slaveMacJ.copyTo(buffStart.data() + 2);

            // Send
            ethernetSocket.send(ef, buffStart);

            // 14) - Expecting message from SlaveJ
            ethernetSocket.receive(this, &ethernetSocket.getInterfaceMac(), &slaveMacJ);

            // Yes means on same switch, No means on different switches
            if (lastMessage == YES) {
                // merge into i
                connectivitySet[i].insert(connectivitySet[j].begin(), connectivitySet[j].end());
                connectivitySet.erase(connectivitySet.begin() + j);
                --j;
            }
        }
    }
}

bool EthernetDiscovery::testPermutation(const MacAddress &gateway, const MacAddress &i, const MacAddress &j, const MacAddress &k) {
    {
        EthernetFrame ef;
        DataBuffer buff(22);

        ef.sourceMac = ethernetSocket.getInterfaceMac();
        buff[0] = SEND_PACKET;
        buff[1] = 13;
        buff[14] = EMPTY;

        // Let SDi send the packet UA1 --> SDj
        ef.destinationMac = i;
        MacAddress::UA1.copyTo(buff.data() + 2); // UA1
        j.copyTo(buff.data() + 8); // SDj
        ethernetSocket.send(ef, buff);
        ethernetSocket.receive(this, &ethernetSocket.getInterfaceMac(), &i);

        // Let SDk send the packet UA1 --> GW
        ef.destinationMac = k;
        MacAddress::UA1.copyTo(buff.data() + 2); // UA1
        gateway.copyTo(buff.data() + 8); // Gateway
        ethernetSocket.send(ef, buff);
        ethernetSocket.receive(this, &ethernetSocket.getInterfaceMac(), &k);

        // Let SDj send the packet SDj --> UA1
        ef.destinationMac = j;
        j.copyTo(buff.data() + 2); // SDj
        MacAddress::UA1.copyTo(buff.data() + 8); // UA1
        buff[1] = 20;
        buff[14] = PROBE;
        buff[15] = 6;
        ethernetSocket.getInterfaceMac().copyTo(buff.data() + 16);
        ethernetSocket.send(ef, buff);
    }

    // Wait for probe directed to us coming from either SDi or SDk and wait for Ready
    MacAddress SDik;
    {
        uint8_t a = 1, b = 1;
        for (uint8_t n = 0; n < 2; ++n) {
            ethernetSocket.receive(this, &ethernetSocket.getInterfaceMac());
            if (lastMessage == READY) {
                --a;
                MacAddress verify;
                verify.copyFrom(ethernetSocket.getReceiveBuffer().data() + ETH_ALEN);
                if (verify != j) {
                    ostringstream oss;
                    oss << "Algorithm 3 - Received Ready from unknown source: " << verify << endl;
                    throw runtime_error(oss.str());
                }
            } else if (lastMessage == YES) {
                --b;
                // Check which slave sent us this message
                SDik.copyFrom(ethernetSocket.getReceiveBuffer().data() + ETH_ALEN);
            }
        }
        if (a != 0 || b != 0) {
            throw runtime_error("Algorithm 3 - Probe (out of sync); Both Yes and Ready not received");
        }
    }

    if (SDik == i) {
        return false;
    } else if (SDik == k) {
        return true;
    } else {
        ostringstream oss;
        oss << "Algorithm 3 - Received Yes from unknown source: " << SDik << endl;
        throw runtime_error(oss.str());
    }
}

// Combination

void EthernetDiscovery::discoverNetwork() {

    // Facts of type {A, B, C, D, ..} NOT-IN-SUBTREE-OF {X, Y}
    vector<pair<set<size_t>, set<size_t>>> factList;
    using FactType = vector<pair<set<size_t>, set<size_t>>>::value_type;

    // Select two slaves from each switch detected when possible
    vector<size_t> slavesToTest;
    for (const set<size_t>& bottomSwitch : connectivitySet) {
        slavesToTest.push_back(*bottomSwitch.begin());
        if (bottomSwitch.size() >= 2) {
            slavesToTest.push_back(*++bottomSwitch.begin());
        }
    }
    sort(slavesToTest.begin(), slavesToTest.end());

    {
        // Standard Facts are all constructed in 'facts' and 'sameSwitch'
        map<set<size_t>, set<size_t>> facts;
        size_t statCount = 0;
        // Start tests and construct conditionals
        {
            // construct set id vector
            vector<vector<size_t>> combs = combinations(slavesToTest, 3);

            for (vector<size_t> &comb : combs) {
                const MacAddress &i = slaveMacs[comb[0]];
                const MacAddress &j = slaveMacs[comb[1]];
                const MacAddress &k = slaveMacs[comb[2]];
                // Test these three
                bool I = true, J = true, K = true;
                bool isSameSwitch = (K = testPermutation(ethernetSocket.getInterfaceMac(), i, j, k)) &&
                                    (J = testPermutation(ethernetSocket.getInterfaceMac(), i, k, j)) &&
                                    (I = testPermutation(ethernetSocket.getInterfaceMac(), j, k, i));
                if (!isSameSwitch) {
                    ++statCount;
                    // Find the one which is not under same switch
                    if (!I) {
                        // I is separate from J/K
                        facts[{comb[1], comb[2]}].insert(comb[0]);
                    } else if (!J) {
                        facts[{comb[0], comb[2]}].insert(comb[1]);
                    } else if (!K) {
                        facts[{comb[0], comb[1]}].insert(comb[2]);
                    }
                }
            }
        }

        for (const map<set<size_t>, set<size_t>>::value_type &fact : facts) {
            factList.push_back({fact.second, fact.first});
        }
    }

    // Derive Distinct Violations
    set<set<size_t>> distinctViolationsSet /*{{}}*/;
    for (const FactType& fact : factList) {
        distinctViolationsSet.insert(fact.first);
    }
    // Sort by set size
    vector<set<size_t>> distinctViolations (distinctViolationsSet.begin(), distinctViolationsSet.end());
    sort(distinctViolations.begin(), distinctViolations.end(),
         [](const set<size_t>& a, const set<size_t>& b) -> bool {
         return a.size() < b.size();
    });

    // Construct tree by adding all switches first
    indexedTopologyTree.clear();
    indexedTopologyTree.setMacArray(slaveMacs);
    /*size_t rootIndex = */indexedTopologyTree.getNewNode(); // Add root, it's always there
    const vector<IndexedTopologyNode>& nodeList = indexedTopologyTree.getNodes();
    for (const set<size_t>& violationList : distinctViolations) {
        // Find node with largest violation list that is a subset of this violation list
        for (ssize_t nodeIndex = nodeList.size() - 1; nodeIndex >= 0; --nodeIndex) {
            const IndexedTopologyNode& node = nodeList[nodeIndex];
            if (Mathematics::SetOperations::setSubsetOf(node.violators, violationList)) {
                // Sets intersect, this means that node.violators isSubsetOf violationList - Pair them together
                size_t switchIndex = indexedTopologyTree.getNewNode();
                indexedTopologyTree.getNode(switchIndex).violators = violationList;
                indexedTopologyTree.addChildToParent(switchIndex, nodeIndex);
                break;
            }
        }
    }

    // Add all the leaf nodes starting from the most restrictive switches
    for (ssize_t nodeIndex = nodeList.size() - 1; nodeIndex >= 0; --nodeIndex) {
        const IndexedTopologyNode& node = nodeList[nodeIndex];
        vector<size_t> childrenToAdd;
        // Compute difference
        set_difference(slavesToTest.begin(), slavesToTest.end(), node.violators.begin(), node.violators.end(), back_inserter(childrenToAdd));
        for (size_t childToAdd : childrenToAdd) {
            // Check that only new children are being added
            if (!indexedTopologyTree.nodeExists(childToAdd)) {
                indexedTopologyTree.addChildToParent(indexedTopologyTree.addNewNode(childToAdd), nodeIndex);
            }
        }
    }

    /*// Sort Fact list in ascending order
    sort(factList.begin(), factList.end(),
         [](const FactType& a, const FactType& b) -> bool {
             return a.first.size() < b.first.size();
         });
    // Construct tree
    // -- Start with a bottom-down approach and therefore, add all nodes to the same root switch
    indexedTopologyTree.clear();
    size_t nodeIndex = indexedTopologyTree.getNewNode();
    for (size_t slaveId : slavesToTest) {
        indexedTopologyTree.addChildToParent(indexedTopologyTree.addNewNode(slaveId), nodeIndex);
    }

    // -- For each rule, manipulate the tree and keep it in a valid state
    for (const FactType& fact : factList) {
        // A fact consists of LHS < RHS, where, LHS nodes cannot be in the same
        // subtree as the RHS nodes
        for (size_t lhsNodeVal : fact.first) {
            indexedTopologyTree.addRule(lhsNodeVal, *fact.second.begin(), *++fact.second.begin());
        }
    }*/

    // Attach remaining nodes to known switches and isolate bottom-layer single-node switches
    for (const set<size_t>& bottomSwitch : connectivitySet) {

        if (bottomSwitch.size() >= 2) {
            size_t parentIndex = indexedTopologyTree.findParentNodeOf(*bottomSwitch.begin());
            // Sanity Check
            if (parentIndex != indexedTopologyTree.findParentNodeOf(*++bottomSwitch.begin())) {
                throw runtime_error("Two nodes supposed to be on the same switch are on different switches");
            }
            if (bottomSwitch.size() >= 3) {
                for (set<size_t>::const_iterator slaveIterator = next(bottomSwitch.begin(), 2);
                     slaveIterator != bottomSwitch.end(); ++slaveIterator) {
                    indexedTopologyTree.addChildToParent(indexedTopologyTree.addNewNode(*slaveIterator), parentIndex);
                }
            }
        } else if (groupSwitches) {
            // Single switch node
            const size_t childIndex = indexedTopologyTree.findNode(*bottomSwitch.begin());
            const size_t parentIndex = indexedTopologyTree.getNode(childIndex).parent;

            // Check whether there is another leaf before isolating
            bool needToIsolate = false;
            for (size_t pcIndex : indexedTopologyTree.getNode(parentIndex).children) {
                if (pcIndex != childIndex && indexedTopologyTree.getNode(pcIndex).isLeaf()) {
                    needToIsolate = true;
                    break;
                }
            }

            if (needToIsolate) {
                const size_t newSwitch = indexedTopologyTree.getNewNode();

                // Move without recomputing violation sets - no need at this stage
                // Additionally, it will change nothing since these are new elements
                indexedTopologyTree.getNode(parentIndex).deleteChild(childIndex);
                indexedTopologyTree.addChildToParent(childIndex, newSwitch);
                indexedTopologyTree.addChildToParent(newSwitch, parentIndex);

                //indexedTopologyTree.moveNodeToNode(newSwitch, parentIndex);
                //indexedTopologyTree.moveNodeToNode(childIndex, newSwitch);
            }
        }
    }
}

Util::NodePt EthernetDiscovery::getToplogyTree() {

    // Algorithm 1
    getAllDevices();

    if (isPingBased) {
        Matrix<float> rttMatrix = startPingBasedDiscovery();
        Mathematics::Matrix<uint32_t> hopCountMatrix = EthernetDiscovery::rttToHopCount(rttMatrix);
        vector<size_t> parent = EthernetDiscovery::hopCountToTopology(hopCountMatrix);
        return getTreeFromParentBasedIndexTree(parent);
    } else {

        // Algorithm 2
        partitionBottomLayer();

        // Algorithm 4 - Our version of the idea
        discoverNetwork();

        return indexedTopologyTree.toTree();
    }
}

void EthernetDiscovery::slave() {
    ethernetSocket.receive(this);
}

template <class T>
vector<vector<T>> EthernetDiscovery::combinations(const std::vector<T> &elems, size_t k) {
    vector<vector<T>> ret;
    if (elems.size() < k) {
        return ret;
    }

    // Init indices
    vector<size_t> index (k);

    // Start
    ssize_t pos = 0;
    do {
        // Reset if required
        for (size_t i = static_cast<size_t>(pos + 1); i < k; ++i) {
            index[i] = index[i - 1] + 1;
        }

        // Add this combination
        vector<T> s;
        for (size_t i = 0; i < k; ++i) {
            s.push_back(elems[index[i]]);
        }
        ret.push_back(s);

        // Move indices
        for (pos = k - 1; pos >=0; --pos) {
            // Check if we can increment
            if (index[pos] < elems.size() - (k - pos)) {
                ++index[pos];
                break;
            }
        }
    } while(pos >= 0);

    return ret;
}

vector<size_t> EthernetDiscovery::hopCountToTopology(const Matrix<uint32_t> &hopMatrix) {

    if (hopMatrix.getColumns() == 0) {
        return {0};
    }

    unique_ptr<Matrix<uint32_t>> workHCPt (new Matrix<uint32_t>(hopMatrix));
    size_t N = hopMatrix.getColumns();
    vector<size_t> nodeNum (N);
    vector<size_t> parent (N);
    iota(nodeNum.begin(), nodeNum.end(), 0);
    size_t newSwitch = N;

    uint32_t largestD;
    do {
        const Matrix<uint32_t>& workHC = *workHCPt;

        // Find max distance value in matrix
        largestD = 0;
        for (size_t r = 0; r < workHC.getRows(); ++r) {
            for (size_t c = 0; c < workHC.getColumns(); ++c) {
                if (largestD < workHC(r, c)) {
                    largestD = workHC(r, c);
                }
            }
        }

        // Base Case
        if (largestD == 1) {
            // Last switch left
            for (size_t nNum : nodeNum) {
                parent[nNum] = newSwitch;
            }
            parent.push_back(newSwitch);
        } else {
            // More than two switches left

            // Find a node n such that there exists a j where WorkHC(n, j) == largestD
            size_t n = 0;
            {
                bool notFound = true;
                for (size_t r = 0; r < workHC.getRows() && notFound; ++r) {
                    for (size_t c = 0; c < workHC.getColumns() && notFound; ++c) {
                        if (workHC(r, c) == largestD) {
                            notFound = false;
                            n = r;
                        }
                    }
                }
                if (notFound) {
                    throw runtime_error("HopToTopology: No node n found");
                }
            }

            // Find all nodes whose distances to n are 1; Let the set of nodes be A
            set<size_t> A;
            A.insert(n);
            for (size_t c = 0; c < workHC.getColumns(); ++c) {
                if (workHC(n, c) == 1) {
                    A.insert(c);
                }
            }

            // Set parent switch for nodes in set A to be the new switch
            for (size_t a : A) {
                parent[nodeNum[a]] = newSwitch;
            }

            // Reduce the tree by removing all nodes in A and adding a new leaf node newSwitch
//            nodeNum.erase(remove_if(nodeNum.begin(), nodeNum.end(),
//                                    [&A](uint32_t val) -> bool { return A.find(val) != A.end(); }), nodeNum.end());
            {
                size_t i = 0;
                for (size_t a : A) {
                    nodeNum.erase(nodeNum.begin() + (a - i++));
                }
            }
            nodeNum.push_back(newSwitch);
            parent.push_back(newSwitch++);

            // Recompute WorkHC
            unique_ptr<Matrix<uint32_t>> tempWorkHCPt (new Matrix<uint32_t>(nodeNum.size(), nodeNum.size()));
            Matrix<uint32_t> &tempWorkHC = *tempWorkHCPt;
            auto rIterator = A.begin();
            for (size_t r = 0, rIndex = 0; r < workHC.getRows(); ++r) {
                if (rIterator != A.end() && r == *rIterator) {
                    // Ignore this row
                    ++rIterator;
                } else {
                    auto cIterator = A.begin();
                    for (size_t c = 0, cIndex = 0; c < workHC.getColumns(); ++c) {
                        if (cIterator != A.end() && c == *cIterator) {
                            // Ignore this column
                            ++cIterator;
                        } else {
                            // Valid column/Row by cIndex
                            tempWorkHC(rIndex, cIndex) = workHC(r, c);
                            ++cIndex;
                        }
                    }
                    ++rIndex;
                }
            }

            // Update last row and column corresponding to newSwitch
            auto rcIterator = A.begin();
            for (size_t rc = 0, rcIndex = 0; rc < workHC.getColumns(); ++rc) {
                if (rcIterator != A.end() && rc == *rcIterator) {
                    // Ignore this column
                    ++rcIterator;
                } else {
                    // Valid column/Row by cIndex
                    tempWorkHC(rcIndex, tempWorkHC.getColumns() - 1) = tempWorkHC(tempWorkHC.getRows() - 1, rcIndex) =
                            workHC(n, rc) - 1;
                    ++rcIndex;
                }
            }

            // Assign new matrix
            workHCPt = move(tempWorkHCPt);
        }
    } while (largestD > 1);

    return parent;
}

// Data Clustering
Matrix<uint32_t> EthernetDiscovery::rttToHopCount(const Matrix<float> &rttMatrix) const {

    if (rttMatrix.getColumns() < 2) {
        return Matrix<uint32_t>(rttMatrix.getRows(), rttMatrix.getColumns());
    }

    // Store matrix in array
    vector<float> rttValues;
    for (size_t r = 0; r < rttMatrix.getRows(); ++r) {
        for (size_t c = 0; c < rttMatrix.getColumns(); ++c) {
            if (r != c) {
                rttValues.push_back(rttMatrix(r, c));
            }
        }
    }
    sort(rttValues.begin(), rttValues.end());
    //float measurementNoise = numRTT * 0.001f;
    size_t numClass = 0;

    // Needed struct
    struct DataClass {
        float lowerBound;
        float upperBound;
        uint32_t hopCount;

        float center() const {
            return lowerBound + 0.5f * (upperBound - lowerBound);
        }

        bool contains(float val) const {
            return lowerBound <= val && val <= upperBound;
        }

        // For debug info
        string toString() const {
            return "lower: " + to_string(lowerBound) + " upper:" + to_string(upperBound);
        }
    };

    // Initial Clustering
    vector<DataClass> dataClasses;
    dataClasses.push_back({rttValues[0], 0.f});
    float prev = rttValues[0];
    for (size_t i = 0; i < rttValues.size(); ++i) {
        if (rttValues[i] - prev < pingParameters.measurementNoise) {
            prev = rttValues[i]; // expand the class
        } else {
            dataClasses[numClass].upperBound = prev;
            ++numClass;
            dataClasses.push_back({rttValues[i], 0.f});
            prev = rttValues[i];
        }
    }
    dataClasses[numClass].upperBound = prev;

    // Further Clustering
    bool hasMerged;
    float smallestInterClassGap = 0.f;
    do {
        hasMerged = false;
        for (size_t i = 0; i < dataClasses.size() - 1; ++i) {
            const float intraClassI = dataClasses[i].upperBound - dataClasses[i].lowerBound;
            const float interClassI1 = dataClasses[i].center(); //dataClasses[i].lowerBound + 0.5f * intraClassI;
            const float interClassI2 = dataClasses[i + 1].center(); //dataClasses[i+1].lowerBound + 0.5f * (dataClasses[i+1].upperBound - dataClasses[i+1].lowerBound);
            const float interClassI = interClassI2 - interClassI1;

            // Need to guarantee that inter >= interThreshold * intra .. otherwise merge
            if (intraClassI > interClassI / pingParameters.interThresholdCoefficient) {
                // Merge classes
                dataClasses[i].upperBound = dataClasses[i+1].upperBound;
                dataClasses.erase(dataClasses.begin() + (i + 1));
                --i;
                hasMerged = true;
            }

            // Find smallest gap
            if (i == 0) {
                smallestInterClassGap = interClassI;
            } else {
                if (smallestInterClassGap > interClassI) {
                    smallestInterClassGap = interClassI;
                }
            }
        }
    } while (hasMerged);

    // Calculate hops for each class
    dataClasses[0].hopCount = 1;
    for (size_t i = 1; i < dataClasses.size(); ++i) {
        dataClasses[i].hopCount = dataClasses[i - 1].hopCount +
                static_cast<uint32_t>(
                        floor(( dataClasses[i].center() - dataClasses[i - 1].center() + 0.5f * smallestInterClassGap ) / smallestInterClassGap)
                );
    }

    // Compute HopCount matrix
    Matrix<uint32_t> hopCountMatrix (rttMatrix.getRows(), rttMatrix.getColumns());
    for (size_t r = 0; r < hopCountMatrix.getRows(); ++r) {
        for (size_t c = 0; c < hopCountMatrix.getColumns(); ++c) {
            if (r != c) {
                for (const DataClass &dataClass : dataClasses) {
                    if (dataClass.contains(rttMatrix(r, c))) {
                        hopCountMatrix(r, c) = dataClass.hopCount;
                        break;
                    }
                }
            }
        }
    }

    return hopCountMatrix;
}

Matrix<float> EthernetDiscovery::startPingBasedDiscovery() {
    // Will use full-matrix approach for the time being
    Matrix<float> rttMatrix (slaveMacs.size(), slaveMacs.size());

    DataBuffer buff (2 + sizeof(MacAddress) + 2 * sizeof(float) + 2 * sizeof(uint32_t));

    for (size_t r = 0; r < rttMatrix.getRows(); ++r) {
        for (size_t c = 0; c < rttMatrix.getColumns(); ++c) {
            if (r == c) continue;

            // Define Ethernet Frame
            EthernetFrame ef;
            ef.destinationMac = slaveMacs[r];

            // Setup message
            buff[0] = BEGIN_PING;
            buff[1] = sizeof(MacAddress) + 2 * sizeof(float) + 2 * sizeof(uint32_t);
            slaveMacs[c].copyTo(buff.data() + 2);

            DataEncoder::writeValToNetworkBytes(pingParameters.stdConfidence, buff.data() + 2 + sizeof(MacAddress));
            DataEncoder::writeValToNetworkBytes(pingParameters.confidenceInterval, buff.data() + 2 + sizeof(MacAddress) + sizeof(float));

            DataEncoder::writeValToNetworkBytes(pingParameters.minMeasurements, buff.data() + 2 + sizeof(MacAddress) + 2 * sizeof(float));
            DataEncoder::writeValToNetworkBytes(pingParameters.maxMeasurements, buff.data() + 2 + sizeof(MacAddress) + 2 * sizeof(float) + sizeof(uint32_t));

            // Send message
            ethernetSocket.send(ef, buff);

            // Receive message
            ethernetSocket.receive(this, &ethernetSocket.getInterfaceMac(), &ef.destinationMac);
            if (lastMessage != END_PING) {
                throw runtime_error("Out of Sync - Expected END_PING and received: " + to_string(static_cast<uint32_t>(lastMessage)));
            }
            rttMatrix (r, c) = pingTime;
        }
    }

    return rttMatrix;
}

void EthernetDiscovery::setPingParameters(const EthernetDiscovery::PingParameters &p_pingParameters) {
    if (p_pingParameters.minMeasurements == 0 || p_pingParameters.minMeasurements > p_pingParameters.maxMeasurements) {
        throw runtime_error("Invalid Ping Parameters Values");
    }
    pingParameters = p_pingParameters;
    isPingBased = true;
}

void EthernetDiscovery::setGroupedSwitches(bool p_groupSwitches) {
    groupSwitches = p_groupSwitches;
}

Util::NodePt EthernetDiscovery::getTreeFromParentBasedIndexTree(const std::vector<size_t> &parentBasedIndexTree) const {
    // Find root
    size_t rootIndex;
    for (rootIndex = 0; rootIndex < parentBasedIndexTree.size(); ++rootIndex) {
        if (rootIndex == parentBasedIndexTree[rootIndex]) {
            break;
        }
    }
    // Find
    return getTreeFromParentBasedIndexTree(parentBasedIndexTree, rootIndex);
}

Util::NodePt EthernetDiscovery::getTreeFromParentBasedIndexTree(const std::vector<size_t> &parentBasedIndexTree,
                                                                size_t nodeIndex) const {
    using namespace Util;

    if (nodeIndex < slaveMacs.size()) { // Leaf case
        return NodePt(new Leaf(slaveMacs.at(nodeIndex)));
    } else {
        NodePt node (new Node());
        //        static_cast<Node *>(node.get())->addChild(child->toTree());
        for(size_t i = 0; i < parentBasedIndexTree.size(); ++i) {
            if (i != nodeIndex && parentBasedIndexTree[i] == nodeIndex) {
                // i is a child node
                static_cast<Node *>(node.get())->addChild(getTreeFromParentBasedIndexTree(parentBasedIndexTree, i));
            }
        }
        return node;
    }
}

void EthernetDiscovery::terminateSlaves() {
    EthernetFrame ef;
    ef.destinationMac = MacAddress::BroadcastMac;
    ethernetSocket.send(ef, {MessageType::EXIT});
}
