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
#include "EthernetSocket.h"
#include "EthernetDiscovery.h"
#include "LinuxNetworkInterface.h"
#include "SimulatedNetworkInterface.h"
#include "SwitchNode.h"

/* Adapted from https://gist.github.com/austinmarton/1922600
 * and     from https://gist.github.com/austinmarton/2862515
 */

using namespace std;
using namespace Network;

int main(int argc, char *argv[])
{

    string interfaceName = DEFAULT_IF;
    bool isSender = true;
    bool help = false;

    // name, hasArg, flag, val
    option options[] = {
            {"interface", required_argument, nullptr, 'i'},
            {"sender",  no_argument, nullptr, 's'},
            {"receiver",  no_argument, nullptr, 'r'},
            {"help",   no_argument,       nullptr, 'h'}
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
            case 'r':
                isSender = false;
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
        << "\t--receiver - Sets as receiver" << endl
        << "\t--help - Shows this Usage Information" << endl;
        return EXIT_SUCCESS;
    }

    try {

        //cout << "Sizes: EF: " << sizeof(EthernetFrame) << ", MAC: " << sizeof(MacAddress) << endl;
        //cout << "Sizes Original: EF " << sizeof(ether_header) << endl;
        SimulatedNetworkInterface simulatedNetworkInterface;

        LinuxNetworkInterface linuxNetworkInterface;
        EthernetSocket es (interfaceName, linuxNetworkInterface);
        EthernetDiscovery ed (es);

        if (isSender) {
            cout << "Starting up as Master" << endl;
            /*EthernetFrame ef;
            ef.destinationMac = MacAddress::GetBroadcastMac();
            ef.setEtherType(ETH_P_IP);
            //ef.etherType = ;
            es.send(ef, vector<u_int8_t>(1, 65)); */
            //send_frame(interfaceName.c_str(), destMac, "Hello");

            ed.master();
        } else {
            cout << "Starting up as Slave" << endl;
            ed.slave();
            //es.recv();
            //recv_frame(interfaceName.c_str(), destMac);
        }

    } catch(const exception& ex) {
        cout << ex.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
