//
// Created by kevin on 6/18/16.
//

#ifndef NETWORK_DISCOVERY_INTERFACE_H
#define NETWORK_DISCOVERY_INTERFACE_H

#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

namespace Network {
    class INetworkInterface {
    public:
        virtual ~INetworkInterface();

        // methods
        virtual int socket(int domain, int type, int protocol) = 0;
        virtual int ioctl (int fd, unsigned long request, void* args) = 0;
        virtual int close (int fd) = 0;
        virtual int setsockopt (int fd, int level, int optname,
                                const void *optval, socklen_t optlen) = 0;
        virtual ssize_t sendto (int fd, const void *buf, size_t n,
                                int flags, const struct sockaddr* addr,
                                socklen_t addr_len) = 0;
        virtual ssize_t recvfrom (int fd, void * buf, size_t n,
                          int flags, struct sockaddr* addr,
                          socklen_t * addr_len) = 0;
    };
}


#endif //NETWORK_DISCOVERY_INTERFACE_H
