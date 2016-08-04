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
#include "NetworkNode/SwitchNode.h"

/* Adapted from https://gist.github.com/austinmarton/1922600
 * and     from https://gist.github.com/austinmarton/2862515
 */

using namespace std;
using namespace Network;

int main(int argc, char *argv[])
{

    string interfaceName = DEFAULT_IF;
    string pathToTopology;
    bool isSender = true;
    bool isPingBased = false;
    bool isVirtual = false;
    bool help = false;

    // name, hasArg, flag, val
    option options[] = {
            {"interface", required_argument, nullptr, 'i'},
            {"sender",  no_argument, nullptr, 's'},
            {"ping",  no_argument, nullptr, 'p'},
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
                break;
            case 'r':
                isSender = false;
                break;
            case 'v':
                isVirtual = true;
                pathToTopology = optarg;
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
        << "\t--ping   - Uses the ping-hopcount algorithm for discovery" << endl
        << "\t--receiver - Sets as receiver" << endl
        << "\t--virtual=[netTopology.xml] - Simulate master and receivers using a virtual network topology" << endl
        << "\t--help - Shows this Usage Information" << endl;
        return EXIT_SUCCESS;
    }

    try {

        if (isPingBased) {
            cout << "Warning: This algorithm is still under construction - using Mock Data as input" << endl;
            Mathematics::Matrix<float> rttMatrix (6, 6);
            vector<vector<float>> rttArr {
                    {0.f, .131f, .157f, .156f, .173f, .175f},
                    {.134f, 0.f, .159f, .158f, .176f, .177f},
                    {.160f, .155f, 0.f, .132f, .158f, .159f},
                    {.159f, .159f, .132f, 0.f, .159f, .159f},
                    {.176f, .176f, .159f, .159f, 0.f, .132f},
                    {.176f, .175f, .159f, .156f, .132f, 0.f}
            };
            for (size_t r = 0; r < rttArr.size(); ++r) {
                for (size_t c = 0; c < rttArr[r].size(); ++c) {
                    rttMatrix (r, c) = rttArr[r][c];
                }
            }

            cout << "RTT Matrix:" << endl << rttMatrix << endl;
            Mathematics::Matrix<uint32_t> hopCountMatrix = EthernetDiscovery::rttToHopCount(rttMatrix);
            cout << "Hop Count Matrix: " << endl << hopCountMatrix << endl;
            vector<size_t> parent = EthernetDiscovery::hopCountToTopology(hopCountMatrix);
            cout << "parent:" << endl;
            for (size_t i = 0; i < parent.size(); ++i) {
                cout << i << ") " << parent[i] << endl;
            }
            return 0;
        }

        if (isVirtual) {
            NetworkNodeFactory netNodeFactory;
            SimulatedNetworkInterface simulatedNetworkInterface (netNodeFactory.make(pathToTopology));
            EthernetSocket esMaster (interfaceName, simulatedNetworkInterface);
            EthernetDiscovery edMaster (esMaster);

            vector<thread> threads;
            vector<EthernetSocket> es;
            vector<EthernetDiscovery> ed;
            const size_t numDevices = simulatedNetworkInterface.getNumNetDevices() - 1;
            es.reserve(numDevices);
            ed.reserve(numDevices);
            threads.reserve(numDevices);
            for (size_t i = 0; i < numDevices; ++i) {
                es.emplace_back(interfaceName, simulatedNetworkInterface);
                ed.emplace_back(es[i]);
                threads.emplace_back(&EthernetDiscovery::slave, ed[i]);
            }
            edMaster.master();
            for (size_t i = 0; i < threads.size(); ++i) {
                threads[i].join();
            }
        } else {
            LinuxNetworkInterface linuxNetworkInterface;
            EthernetSocket es (interfaceName, linuxNetworkInterface);
            EthernetDiscovery ed (es);

            if (isSender) {
                ed.master();
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
