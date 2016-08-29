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

int main(int argc, char *argv[])
{

    string interfaceName = DEFAULT_IF;
    string virtualTopologyParameters;
    // StdConfidence, Confidence Interval Value, measurement noise(first pass),interThreshold Coefficient
    string strPingParameters = "2,0.001,0.008,3,2,1024";
    bool isSender = true;
    bool isPingBased = false;
    bool isVirtual = false;
    bool help = false;

    // name, hasArg, flag, val
    option options[] = {
            {"interface", required_argument, nullptr, 'i'},
            {"sender",  no_argument, nullptr, 's'},
            {"ping",  optional_argument, nullptr, 'p'},
            {"receiver",  no_argument, nullptr, 'r'},
            {"virtual",  required_argument, nullptr, 'v'},
            {"help",   no_argument,       nullptr, 'h'},
            {nullptr, 0,                  nullptr, 0} // Last entry must be all zeros
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
        << "\t--virtual=[netTopology.xml,masterIndex,masterRunCount] - Simulate master and receivers using a virtual network topology" << endl
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

            NetworkNodeFactory netNodeFactory;
            SimulatedNetworkInterface simulatedNetworkInterface (netNodeFactory.make(vTopologyParameters[0]));

            const size_t masterIndex = vTopologyParameters.size() > 1 ? static_cast<uint32_t>(stoi(vTopologyParameters[1])) : 0;
            const size_t masterRunCount = vTopologyParameters.size() > 2 ? static_cast<uint32_t>(stoi(vTopologyParameters[2])) : 1;
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
            }
            for (size_t i = 0; i < masterRunCount; ++i) {
                cout << "Master Run: #" << (i + 1) << endl;
                try {
                    ed[masterIndex].master(isPingBased);
                } catch (const exception& ex) {
                    cout << "Virtual Master Catch Block: " << ex.what() << endl;
                }
                ed[masterIndex].clear();
            }
            for (size_t i = 0; i < threads.size(); ++i) {
                threads[i].join();
            }
        } else {
            LinuxNetworkInterface linuxNetworkInterface;
            EthernetSocket es (interfaceName, linuxNetworkInterface);
            EthernetDiscovery ed (es);

            if (isSender) {
                if (isPingBased) {
                    ed.setPingParameters(pingParameters);
                }
                ed.master(isPingBased);
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
