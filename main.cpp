#include <iostream>
#include <arpa/inet.h>

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <unistd.h>

#include <getopt.h>
#include <cstring>

#define DEFAULT_IF	"eth0"
#define BUF_SIZ		1024

#include <vector>
#include <stdexcept>
#include <thread>
#include <NetworkNode/NetworkNodeFactory.h>
#include "Ethernet/EthernetSocket.h"
#include "Ethernet/EthernetDiscovery.h"
#include "NetworkInterface/LinuxNetworkInterface.h"
#include "NetworkInterface/SimulatedNetworkInterface.h"
#include "Util/StringOperations.h"

/* Adapted from https://gist.github.com/austinmarton/1922600
 * and     from https://gist.github.com/austinmarton/2862515
 */

using namespace std;
using namespace Network;
using namespace Util;

NodePt runVirtualTopology( bool isPingBased, const EthernetDiscovery::PingParameters& pingParameters, bool isGrouped,
                          const string& interfaceName,
                          SimulatedNetworkInterface& simulatedNetworkInterface,
                          size_t masterIndex,
                          size_t masterRunCount)
{
    const size_t numDevices = simulatedNetworkInterface.getNumNetDevices();

    if (masterIndex >= numDevices) {
        throw runtime_error ("Master index out of range");
    }

    // List Devices
    for (size_t i = 0; i < numDevices; ++i) {
        cout << i << ") " << simulatedNetworkInterface.getNetDevices()[i]->getMacAddress();
        if (i == masterIndex) {
            cout << " (Master)";
        }
        cout << endl;
    }

    vector<thread> threads;
    vector<EthernetSocket> es;
    vector<EthernetDiscovery> ed;
    es.reserve(numDevices);
    ed.reserve(numDevices);
    threads.reserve(numDevices - 1);
    for (size_t i = 0; i < numDevices; ++i) {
        es.emplace_back(interfaceName, simulatedNetworkInterface);
        ed.emplace_back(es[i]);
        if (i != masterIndex) {
            threads.emplace_back(&EthernetDiscovery::slave, ed[i]);
        }
    }
    if (isPingBased) {
        ed[masterIndex].setPingParameters(pingParameters);
    } else {
        ed[masterIndex].setGroupedSwitches(isGrouped);
    }
    string virtException;
    NodePt  topologyTree;
    for (size_t i = 0; i < masterRunCount; ++i) {
        cout << "Master Run: #" << (i + 1) << endl;
        try {
            topologyTree = ed[masterIndex].getToplogyTree();
        } catch (const exception& ex) {
            virtException = ex.what();
            cout << "Virtual Master Catch Block: " << ex.what() << endl;
            ed[masterIndex].terminateSlaves();
        }
        ed[masterIndex].clear();
    }
    for (size_t i = 0; i < threads.size(); ++i) {
        threads[i].join();
    }
    if (!virtException.empty()) {
        throw runtime_error(virtException);
    }

    return topologyTree;
}

int main(int argc, char *argv[])
{

    string interfaceName = DEFAULT_IF;
    string virtualTopologyParameters;
    // StdConfidence, Confidence Interval Value, measurement noise(first pass),interThreshold Coefficient
    string strPingParameters = "2,0.001,0.008,3,2,1024";
    uint32_t sendDelayAmount = 0;
    bool isSender = true;
    bool isPingBased = false;
    bool isVirtual = false;
    bool isGrouped = false;
    bool help = false;

    // name, hasArg, flag, val
    option options[] = {
            {"interface", required_argument, nullptr, 'i'},
            {"sender",    no_argument,       nullptr, 's'},
            {"ping",      optional_argument, nullptr, 'p'},
            {"receiver",  no_argument,       nullptr, 'r'},
            {"delay",     optional_argument, nullptr, 'd'},
            {"virtual",   required_argument, nullptr, 'v'},
            {"grouped",   no_argument,       nullptr, 'g'},
            {"help",      no_argument,       nullptr, 'h'},
            {nullptr,     0,                 nullptr,  0 } // Last entry must be all zeros
    };

    int c;
    int opIndex;
    while ((c = getopt_long_only(argc, argv, "", options, &opIndex)) != -1) {
        switch (c) {
            case 'i':
                interfaceName = optarg;
                break;
            case 's':
                isSender = true;
                break;
            case 'd':
            {
                int sDA = optarg ? stoi(optarg) : 32;
                if (sDA >= 0) {
                    sendDelayAmount = static_cast<uint32_t>(sDA);
                }
            }
                break;
            case 'p':
                isPingBased = true;
                strPingParameters = optarg ? optarg : strPingParameters;
                break;
            case 'r':
                isSender = false;
                break;
            case 'v':
                isVirtual = true;
                virtualTopologyParameters = optarg;
                break;
            case 'g':
                isGrouped = true;
                break;
            case 'h':
                help = true;
                break;
            case ':':
            case '?':
                help = true;
                cout << "Unknown option or missing argument: "
                << (char)c << " : " << optarg << endl;
            default:
                help = true;
                cout << "Unknown Error while Parsing Arguments- " <<
                (char)c << " : " << optarg << endl;
        }
    }

    if (help)
    {
        cout << "Usage:" << endl
        << "\t--interface=[interface name], default: " << DEFAULT_IF << endl
        << "\t--sender - Sets as sender" << endl
        << "\t--ping=[stdConf,confInterval,noise,interThresholdCoefficient,minPings,maxPings] - Uses the ping-hopcount algorithm for discovery, default: " << strPingParameters << endl
        << "\t--receiver - Sets as receiver" << endl
        << "\t--delay=[32] - (HotFix) Delays the send Method, fixing packet re-ordering. Only has effect on non-virtual runs" << endl
        << "\t--virtual=[Xml:netTopology.xml,masterIndex,masterRunCount] - Simulate getToplogyTree and receivers using a virtual network topology" << endl
        << "\t--grouped - Group leaf nodes on the same switch first before performing Algorithm 3/4 (incomplete graph resolution for this)" << endl
        << "\t--help - Shows this Usage Information" << endl;
        return EXIT_SUCCESS;
    }

    try {

        // Set Ping Parameters
        vector<string> vPingParameters = Util::StringOperations::split(strPingParameters, ',');
        if (vPingParameters.size() != 6) {
            throw runtime_error("Invalid Number of Ping Parameters. Expected 4, have: " + to_string(vPingParameters.size()));
        }
        EthernetDiscovery::PingParameters pingParameters {
                stof(vPingParameters[0]),
                stof(vPingParameters[1]),
                stof(vPingParameters[2]),
                stof(vPingParameters[3]),
                static_cast<uint32_t>(stoi(vPingParameters[4])),
                static_cast<uint32_t>(stoi(vPingParameters[5]))
        };

        if (isVirtual) {

            // Parse virtual topology parameters
            vector<string> vTopologyParameters = Util::StringOperations::split(virtualTopologyParameters, ',');
            if (vTopologyParameters.size() == 0) {
                throw runtime_error("Invalid virtual topology parameters passed in argument");
            }

            const size_t masterIndex = vTopologyParameters.size() > 1 ? static_cast<uint32_t>(stoi(vTopologyParameters[1])) : 0;
            const size_t masterRunCount = vTopologyParameters.size() > 2 ? static_cast<uint32_t>(stoi(vTopologyParameters[2])) : 1;

            SimulatedNetworkInterface simulatedNetworkInterface (NetworkNodeFactory().make(vTopologyParameters[0]));

            NodePt originalTopology = simulatedNetworkInterface.getNetworkTree()->toTree();
            // Remove master from original topology, as this cannot (with all methods) be detected
            originalTopology->deleteValue(simulatedNetworkInterface.getNetDevices().at(masterIndex)->getMacAddress());
//            Leaf * leaf = originalTopology->findNode(simulatedNetworkInterface.getNetDevices().at(5)->getMacAddress());
//            originalTopology = leaf->getParent()->makeRoot(move(originalTopology));

            NodePt detectedTopology = runVirtualTopology(isPingBased, pingParameters, isGrouped, interfaceName, simulatedNetworkInterface, masterIndex, masterRunCount);

            originalTopology->toDotFile("original-topology.dot");
            detectedTopology->toDotFile("detected-topology.dot");

            cout << ((*originalTopology == *detectedTopology) ? "Topologies are exactly equal" : "Topologies are not exactly equal") << endl;

        } else {
            LinuxNetworkInterface linuxNetworkInterface;
            // Ping based check here works only for the Master - the slaves will never know this.
            EthernetSocket es (interfaceName, linuxNetworkInterface, isPingBased ? 0 : sendDelayAmount);
            EthernetDiscovery ed (es);

            if (isSender) {
                if (isPingBased) {
                    ed.setPingParameters(pingParameters);
                } else {
                    ed.setGroupedSwitches(isGrouped);
                }
                ed.getToplogyTree()->toDotFile("detected-topology.dot");;
            } else {
                ed.slave();
            }
        }

    } catch(const exception& ex) {
        cout << ex.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
