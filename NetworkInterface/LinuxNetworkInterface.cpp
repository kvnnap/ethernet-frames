//
// Created by kevin on 6/18/16.
//

#include "LinuxNetworkInterface.h"

int Network::LinuxNetworkInterface::socket(int domain, int type, int protocol) {
    return ::socket(domain, type, protocol);
}

int Network::LinuxNetworkInterface::ioctl(int fd, unsigned long request, void *args) {
    return ::ioctl(fd, request, args);
}

int Network::LinuxNetworkInterface::close(int fd) {
    return ::close(fd);
}

int Network::LinuxNetworkInterface::setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen) {
    return ::setsockopt(fd, level, optname, optval, optlen);
}

ssize_t Network::LinuxNetworkInterface::sendto(int fd, const void *buf, size_t n, int flags,
                                               const struct sockaddr *addr, socklen_t addr_len) {
    return ::sendto(fd, buf, n, flags, addr, addr_len);
}

ssize_t Network::LinuxNetworkInterface::recvfrom(int fd, void *buf, size_t n, int flags, struct sockaddr *addr,
                                                 socklen_t *addr_len) {
    return ::recvfrom(fd, buf, n, flags, addr, addr_len);
}











