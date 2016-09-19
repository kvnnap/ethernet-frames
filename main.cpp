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
                          size_t numDevices,
                          bool terminateSlaves,
                          size_t masterIndex,
                          size_t masterRunCount)
{
    // Lists for threads, ethernet socket and ethernet discovery for each thread but based on single interface instance
    vector<thread> threads;
    vector<EthernetSocket> es;
    vector<EthernetDiscovery> ed;

    // Reserving is a must
    es.reserve(numDevices);
    ed.reserve(numDevices);
    threads.reserve(numDevices - 1);

    // Assign objects and initialise slave threads
    for (size_t i = 0; i < numDevices; ++i) {
        es.emplace_back(interfaceName, simulatedNetworkInterface);
        ed.emplace_back(es[i]);
        if (i != masterIndex) {
            threads.emplace_back(&EthernetDiscovery::slave, ed[i]);
        }
    }

    // Set Master's ethernet discovery configuration
    if (isPingBased) {
        ed[masterIndex].setPingParameters(pingParameters);
    } else {
        ed[masterIndex].setGroupedSwitches(isGrouped);
    }

    // Run the simulation
    string strException;
    NodePt  topologyTree;

    try {
        for (size_t i = 0; i < masterRunCount; ++i) {
            //cout << "Master Run: #" << (i + 1) << endl;
            topologyTree = ed[masterIndex].getToplogyTree();
        }
    } catch (const exception& ex) {
        cout << "Virtual Master Catch Block (will throw again): " << ex.what() << endl;
        strException = ex.what();
    }

    // Send Broadcast message so that other threads exit from the slave() method normally
    if (terminateSlaves) {
        ed[masterIndex].terminateSlaves();
    }

    // Collect threads
    for (size_t i = 0; i < threads.size(); ++i) {
        threads[i].join();
    }

    // If exceptions were encountered, throw them back here
    if (!strException.empty()) {
        throw runtime_error(strException);
    }

    // Return the detected network topology tree
    return topologyTree;
}

void saveCompareAndPrintResult(NodePt& actualTopology, NodePt& detectedTopology, size_t run, size_t& pass, size_t& fail,
                        const string& actualTopologyFileName, const string& detectedTopologyFileName) {
    actualTopology->toDotFile(to_string(run) + "-" + actualTopologyFileName);
    detectedTopology->toDotFile(to_string(run) + "-" + detectedTopologyFileName);
    const AbstractNode * const temp = detectedTopology.get();
    cout << "#" << run << " - ";
    if (actualTopology->unrootedWeakEquality(detectedTopology)) {
        cout << "Topologies are weakly equal";
        ++pass;
    } else {
        cout << "Topologies are not weakly equal";
        ++fail;
    }
    cout << endl;
    // Save re-rooted file only if the rerooting actually occurred
    if (detectedTopology.get() != temp) {
        detectedTopology->toDotFile(to_string(run) + "-rerooted-" + detectedTopologyFileName);
    }
}

int main(int argc, char *argv[])
{
    string interfaceName = DEFAULT_IF;
    string virtualTopologyParameters;
    // StdConfidence, Confidence Interval Value, measurement noise(first pass),interThreshold Coefficient
    string strPingParameters = "2,0.001,0.008,3,2,1024";
    uint32_t sendDelayAmount = 0;
    size_t testRuns = 0;
    bool isSender = true;
    bool isPingBased = false;
    bool isVirtual = false;
    bool isGrouped = true;
    bool terminateSlaves = false;
    bool help = false;

    // name, hasArg, flag, val
    option options[] = {
            {"interface", required_argument, nullptr, 'i'},
            {"sender",    no_argument,       nullptr, 's'},
            {"ping",      optional_argument, nullptr, 'p'},
            {"receiver",  no_argument,       nullptr, 'r'},
            {"delay",     optional_argument, nullptr, 'd'},
            {"virtual",   required_argument, nullptr, 'v'},
            {"ungrouped", no_argument,       nullptr, 'u'},
            {"testruns",  required_argument, nullptr, 't'},
            {"quitslaves",no_argument,       nullptr, 'q'},
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
            case 'd': {
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
            case 'u':
                isGrouped = false;
                break;
            case 't': {
                int tRuns = optarg ? stoi(optarg) : 0;
                if (tRuns >= 0) {
                    testRuns = static_cast<size_t>(tRuns);
                }
            }
                break;
            case 'q':
                terminateSlaves = true;
                break;
            case 'h':
                help = true;
                break;
            case ':':
            case '?':
                help = true;
                cout << "Unknown option or missing argument: "
                << (char)c << " : " << (optarg ? optarg : "No Argument Details") << endl;
                break;
            default:
                help = true;
                cout << "Unknown Error while Parsing Arguments- " <<
                (char)c << " : " << (optarg ? optarg : "No Argument Details") << endl;
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
        << "\t--ungrouped - Do not group leaf nodes on the same switch first before performing Algorithm 3/4" << endl
        << "\t--test=[numOfTests] - Tests for eth vs ping" << endl
        << "\t--quitslaves - Exit slaves after finishing" << endl
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

        size_t pass = 0;
        size_t fail = 0;

        if (isVirtual) {

            // Parse virtual topology parameters
            vector<string> vTopologyParameters = Util::StringOperations::split(virtualTopologyParameters, ',');
            if (vTopologyParameters.size() == 0) {
                throw runtime_error("Invalid virtual topology parameters passed in argument");
            }

            const size_t masterIndex = vTopologyParameters.size() > 1 ? static_cast<uint32_t>(stoi(vTopologyParameters[1])) : 0;
            const size_t masterRunCount = vTopologyParameters.size() > 2 ? static_cast<uint32_t>(stoi(vTopologyParameters[2])) : 1;
            if (masterRunCount == 0) {
                throw runtime_error("Invalid virtual topology parameters passed in argument. Master Run Count should be > 0");
            }

            // 0 = validate simulated vs detected
            if (testRuns > 0) {
                for (size_t run = 0; run < testRuns; ++run) {
                    SimulatedNetworkInterface simulatedNetworkInterface(
                            NetworkNodeFactory().make(vTopologyParameters[0]));
                    const size_t numDevices = simulatedNetworkInterface.getNumNetDevices();
                    if (masterIndex >= numDevices) {
                        throw runtime_error("Master index out of range");
                    }

                    NodePt originalTopology = simulatedNetworkInterface.getNetworkTree()->toTree();
                    // Remove master from original topology, as this cannot (with all methods) be detected
                    originalTopology->deleteValue(
                            simulatedNetworkInterface.getNetDevices().at(masterIndex)->getMacAddress());

                    NodePt detectedTopology = runVirtualTopology(isPingBased, pingParameters, isGrouped, interfaceName,
                                                                 simulatedNetworkInterface, numDevices, terminateSlaves,
                                                                 masterIndex, masterRunCount);

                    saveCompareAndPrintResult(originalTopology, detectedTopology, run, pass, fail,
                                              "original-topology.dot", "detected-topology.dot");
                }
                cout << "Pass: " << pass << " Fail: " << fail << " Total: " << (pass + fail)
                     << " Success Rate: " << ((pass * 100.f) / (pass + fail)) << "%" << endl;
            } else {
                SimulatedNetworkInterface simulatedNetworkInterface(
                        NetworkNodeFactory().make(vTopologyParameters[0]));
                const size_t numDevices = simulatedNetworkInterface.getNumNetDevices();
                if (masterIndex >= numDevices) {
                    throw runtime_error("Master index out of range");
                }
                runVirtualTopology(isPingBased, pingParameters, isGrouped, interfaceName,
                                   simulatedNetworkInterface, numDevices, terminateSlaves,
                                   masterIndex, masterRunCount)->toDotFile("detected-topology.dot");
            }

        } else {
            LinuxNetworkInterface linuxNetworkInterface;
            // Ping based check here works only for the Master - the slaves will never know this.
            EthernetSocket es (interfaceName, linuxNetworkInterface, isPingBased ? 0 : sendDelayAmount);
            EthernetDiscovery ed (es);

            if (isSender) {
                // Compare eth vs Ping
                if (testRuns > 0) {
                    for (size_t run = 0; run < testRuns; ++run) {

                        // Get eth result
                        es.setSendDelayAmount(sendDelayAmount);
                        ed.clear();
                        ed.setGroupedSwitches(isGrouped);
                        NodePt ethTree = ed.getToplogyTree();

                        // Get Ping result
                        es.setSendDelayAmount(0);
                        ed.clear();
                        ed.setPingParameters(pingParameters);
                        NodePt pingTree = ed.getToplogyTree();

                        // Print trees and tell result
                        saveCompareAndPrintResult(ethTree, pingTree, run, pass, fail, "eth-topology.dot",
                                                  "ping-topology.dot");
                    }
                    cout << "Pass: " << pass << " Fail: " << fail << " Total: " << (pass + fail)
                         << " Success Rate: " << ((pass * 100.f) / (pass + fail)) << "%" << endl;
                } else {
                    if (isPingBased) {
                        ed.setPingParameters(pingParameters);
                    } else {
                        ed.setGroupedSwitches(isGrouped);
                    }
                    ed.getToplogyTree()->toDotFile("detected-topology.dot");
                }
                // Send Broadcast message so that other threads exit from the slave() method normally
                if (terminateSlaves) {
                    ed.terminateSlaves();
                }
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
