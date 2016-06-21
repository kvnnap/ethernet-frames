//
// Created by kevin on 5/29/16.
//

#include <iostream>
#include <sstream>
#include <numeric>
#include "EthernetDiscovery.h"

using namespace Network;
using namespace std;

EthernetDiscovery::EthernetDiscovery(EthernetSocket &ethernetSocket)
        : ethernetSocket ( ethernetSocket ), lastMessage (EMPTY), testReceived ( false )
{ }

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
                // Add to list - Verify that we are master
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
}

void EthernetDiscovery::partitionBottomLayer() {

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

    connectivitySet.clear();
    // Build Connectivity Set
    for (size_t i = 0; i < slaveMacs.size(); ++i) {
        connectivitySet.push_back({i});
    }

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

    // construct set id vector
    vector<size_t> setIds (connectivitySet.size());
    iota(setIds.begin(), setIds.end(), 0);
    vector<vector<size_t>> combs = combinations(setIds, 3);
    vector<pair<size_t, set<size_t>>> facts;

    // Start tests and construct conditionals
    for (vector<size_t>& comb : combs) {
        const MacAddress& i = slaveMacs[*connectivitySet[comb[0]].begin()];
        const MacAddress& j = slaveMacs[*connectivitySet[comb[1]].begin()];
        const MacAddress& k = slaveMacs[*connectivitySet[comb[2]].begin()];
        // Test these three
        bool I = true, J = true, K = true;
        bool isSameSwitch = (K = testPermutation(ethernetSocket.getInterfaceMac(), i, j, k)) &&
                            (J = testPermutation(ethernetSocket.getInterfaceMac(), i, k, j)) &&
                            (I = testPermutation(ethernetSocket.getInterfaceMac(), j, k ,i));
        if (isSameSwitch) {
            // All are under a same switch
            facts.push_back(make_pair(comb[0], set<size_t>{comb[1], comb[2], comb[3]}));
        } else {
            // Find the one which is not under same switch
            if (!I) {
                // I is separate from J/K
                facts.push_back(make_pair(comb[0], set<size_t>{comb[1], comb[2]}));
            } else if (!J) {
                facts.push_back(make_pair(comb[1], set<size_t>{comb[0], comb[2]}));
            } else if (!K) {
                facts.push_back(make_pair(comb[2], set<size_t>{comb[0], comb[1]}));
            }
        }
    }

    // Make union of all common elements


}

void EthernetDiscovery::master() {

    // Algorithm 1
    getAllDevices();
    // Print vector
    cout << "Received From: " << slaveMacs.size() <<  endl;
    for (size_t i = 0; i < slaveMacs.size(); ++i) {
        cout << i << ") " << slaveMacs[i] << endl;
    }

    // Algorithm 2
    partitionBottomLayer();
    cout << "Connectivity Matrix: " << endl;
    //cout << *connectivityMatrix << endl;
    for (size_t i = 0; i < connectivitySet.size(); ++i) {
        cout << "Set " << i << ": ";
        for (const size_t& node : connectivitySet[i]) {
            cout << node;
            if (&node != &*--connectivitySet[i].end()) {
                cout << ", ";
            }
        }
        cout << endl;
    }

    // Algorithm 3 - Test
    if (slaveMacs.size() >= 3) {
        cout << "Test using first 3: " << endl;
        cout << "0, 1, 2: " << testPermutation(ethernetSocket.getInterfaceMac(), slaveMacs[0], slaveMacs[1], slaveMacs[2]) << endl;
        cout << "0, 2, 1: " << testPermutation(ethernetSocket.getInterfaceMac(), slaveMacs[0], slaveMacs[2], slaveMacs[1]) << endl;
        cout << "1, 2, 0: " << testPermutation(ethernetSocket.getInterfaceMac(), slaveMacs[1], slaveMacs[2], slaveMacs[0]) << endl;
    }

 }

void EthernetDiscovery::slave() {
    ethernetSocket.receive(this);
}

template <class T>
vector<vector<T>> EthernetDiscovery::combinations(const std::vector<T> &elems,
                                                                       size_t k) {
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
