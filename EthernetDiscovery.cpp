//
// Created by kevin on 5/29/16.
//

#include <iostream>
#include "EthernetDiscovery.h"

using namespace Network;
using namespace Mathematics;
using namespace std;

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
            // Act only if this packet was meant for this Master
            if (ef.destinationMac == ethernetSocket.getInterfaceMac()) {
                // 13) Respond with yes or no
                EthernetFrame replyEf;
                replyEf.destinationMac.copyFrom(data + 2); // Master MAC
                replyEf.sourceMac = ethernetSocket.getInterfaceMac();
                ethernetSocket.send(replyEf, {testReceived ? MessageType::YES : MessageType::NO});
                testReceived = false;
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

        case MessageType::EMPTY:
        default:
            return true;
    }

    return true;
}


EthernetDiscovery::EthernetDiscovery(EthernetSocket &ethernetSocket)
        : ethernetSocket ( ethernetSocket ), lastMessage (EMPTY), testReceived ( false )
{ }

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

    connectivityMatrix.reset(new Matrix<uint8_t>(slaveMacs.size(), slaveMacs.size()));

    for(size_t i = 0; i < slaveMacs.size(); ++i) {

        const MacAddress& slaveMacI = slaveMacs[i];

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

        for(size_t j = 0; j < slaveMacs.size(); ++j) {

            const MacAddress& slaveMacJ = slaveMacs[j];

            if (i == j) {
                (*connectivityMatrix)(i, j) = '1';
                continue;
            }

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

            // Yes means of same switch, No means not on different switches
            (*connectivityMatrix)(i, j) = static_cast<uint8_t >(lastMessage == YES ? '1' : '0');
        }
    }
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
    cout << *connectivityMatrix << endl;
 }

void EthernetDiscovery::slave() {
    ethernetSocket.receive(this);
}









