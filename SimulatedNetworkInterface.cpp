//
// Created by kevin on 6/18/16.
//

#include "SimulatedNetworkInterface.h"

using namespace Network;

int SimulatedNetworkInterface::socket(int domain, int type, int protocol) {
    return 0;
}

int SimulatedNetworkInterface::ioctl(int fd, unsigned long request, void *args) {
    return 0;
}

int SimulatedNetworkInterface::close(int fd) {
    return 0;
}

int SimulatedNetworkInterface::setsockopt(int fd, int level, int optname, const void *optval,
                                                   socklen_t optlen) {
    return 0;
}

ssize_t SimulatedNetworkInterface::sendto(int fd, const void *buf, size_t n, int flags,
                                                   const struct sockaddr *addr, socklen_t addr_len) {
    return 0;
}

ssize_t SimulatedNetworkInterface::recvfrom(int fd, void *buf, size_t n, int flags, struct sockaddr *addr,
                                                     socklen_t *addr_len) {
    return 0;
}











