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
#include "EthernetSocket.h"
#include "EthernetDiscovery.h"
#include "NetworkInterface/LinuxNetworkInterface.h"
#include "NetworkInterface/SimulatedNetworkInterface.h"
#include "NetworkInterface/SwitchNode.h"

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

        SimulatedNetworkInterface simulatedNetworkInterface;
        LinuxNetworkInterface linuxNetworkInterface;
        EthernetSocket esMaster (interfaceName, simulatedNetworkInterface);
        EthernetDiscovery edMaster (esMaster);

        if (isSender) {
            cout << "Starting up as Master: Thread ID: " << this_thread::get_id() << endl;

            vector<thread> threads;
            vector<EthernetSocket> es;
            vector<EthernetDiscovery> ed;
            es.reserve(8);
            ed.reserve(8);
            threads.reserve(8);

            for (size_t i = 0; i < 8; ++i) {
                es.emplace_back(interfaceName, simulatedNetworkInterface);
                ed.emplace_back(es[i]);
                threads.emplace_back(&EthernetDiscovery::slave, ed[i]);
            }

//            EthernetSocket es1 (interfaceName, simulatedNetworkInterface);
//            EthernetDiscovery ed1 (es1);
//            EthernetSocket es2 (interfaceName, simulatedNetworkInterface);
//            EthernetDiscovery ed2 (es2);
//            EthernetSocket es3 (interfaceName, simulatedNetworkInterface);
//            EthernetDiscovery ed3 (es3);
//
//            thread t1 (&EthernetDiscovery::slave, ed1);
//            thread t2 (&EthernetDiscovery::slave, ed2);
//            thread t3 (&EthernetDiscovery::slave, ed3);
            edMaster.master();
            for (size_t i = 0; i < threads.size(); ++i) {
                threads[i].join();
            }
//            t1.join();
//            t2.join();
//            t3.join();
        } else {
            cout << "Starting up as Slave" << endl;
            edMaster.slave();
            //es.recv();
            //recv_frame(interfaceName.c_str(), destMac);
        }

    } catch(const exception& ex) {
        cout << ex.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
