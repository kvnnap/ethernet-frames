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

/* Adapted from https://gist.github.com/austinmarton/1922600
 * and     from https://gist.github.com/austinmarton/2862515
 */

void send_frame(const char* ifaceName, uint8_t destMac[ETH_ALEN], const char* msg) {

    int sockfd;
    struct ifreq if_idx;
    struct ifreq if_mac;

    size_t tx_len = 0;
    char sendbuf[BUF_SIZ];
    struct ether_header *eh = (struct ether_header *) sendbuf;
    struct iphdr *iph = (struct iphdr *) (sendbuf + sizeof(*eh));
    struct sockaddr_ll socket_address;
    char ifName[IFNAMSIZ];

    /* Get interface name */
    strncpy(ifName, ifaceName, sizeof(ifName));

    /* Open RAW socket to send on */
    // Why IPPROTO_RAW? Try htons(ETH_P_ALL) later
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
        perror("socket");
        return;
    }

    // could probably merge into one structure
    /* Get the index of the interface to send on */
    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);
    if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
        perror("SIOCGIFINDEX");

    /* Get the MAC address of the interface to send on */
    memset(&if_mac, 0, sizeof(struct ifreq));
    strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
    if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
        perror("SIOCGIFHWADDR");

    /* Construct the Ethernet header */
    memset(sendbuf, 0, BUF_SIZ);
    /* Ethernet header */
    memcpy(eh->ether_shost, if_mac.ifr_hwaddr.sa_data, sizeof(eh->ether_shost));
    memcpy(eh->ether_dhost, destMac, sizeof(eh->ether_dhost));
    /* Ethertype field */
    eh->ether_type = htons(ETH_P_IP);
    tx_len += sizeof(*eh);

    /* Packet data */
    mempcpy(sendbuf + tx_len, msg, strlen(msg));
    tx_len += strlen(msg);

    /* Index of the network device */
    socket_address.sll_ifindex = if_idx.ifr_ifindex;
    /* Address length*/
    socket_address.sll_halen = ETH_ALEN;
    /* Destination MAC */
    memcpy(socket_address.sll_addr, destMac, sizeof(destMac));

    /* Send packet */
    if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(socket_address)) < 0)
        printf("Send failed\n");

    close(sockfd);
}

void recv_frame (const char* ifaceName, uint8_t destMac[ETH_ALEN]) {
    uint8_t buf[BUF_SIZ];
    int sockfd;

    char ifName[IFNAMSIZ];

    /* Get interface name */
    strncpy(ifName, ifaceName, sizeof(ifName));

    /* Header structures */
    struct ether_header *eh = (struct ether_header *) buf;

    /* Open PF_PACKET socket, listening for EtherType ETH_P_IP*/
    // Using ETH_P_ALL instead for EtherType. This is used to filter by EtherType
    if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP))) == -1) {
        perror("listener: socket");
        return;
    }

    /* Bind to device */
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)	{
        perror("SO_BINDTODEVICE");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    ssize_t numBytes;
    printf("listener: Waiting to recvfrom...\n");

    while((numBytes = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL)) != -1) {
        //printf("received: %d bytes\n", numBytes);

        bool us = true;
        for (size_t i = 0; i < sizeof(destMac); ++i) {
            us &= eh->ether_dhost[i] == destMac[i];
        }

        if (us) {
            printf("received: For us!\n");
        }
    }

    close(sockfd);
}

int main(int argc, char *argv[])
{
    using namespace std;

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

    uint8_t destMac[ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    if (isSender) {
        cout << "Starting up as Sender" << endl;
        send_frame(interfaceName.c_str(), destMac, "Hello");
    } else {
        cout << "Starting up as Receiver" << endl;
        recv_frame(interfaceName.c_str(), destMac);
    }

    return EXIT_SUCCESS;
}