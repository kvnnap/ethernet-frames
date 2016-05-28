//
// Created by kevin on 5/27/16.
//

#include "MacAddress.h"

using namespace std;
using namespace Network;

MacAddress::MacAddress()
    : mac ()
{ }

MacAddress::MacAddress(const Mac &p_mac)
    : mac (p_mac)
{ }

const Mac& MacAddress::getMacArray() const {
    return mac;
};

void MacAddress::copyTo(u_int8_t *dest) const {
    copy(mac.data(), mac.data() + mac.size(), dest);
}

void MacAddress::copyFrom(const u_int8_t *src) {
    copy(src, src + mac.size(), mac.data());
}

void MacAddress::setMacArray(const Mac &p_mac) {
    mac = p_mac;
}

void MacAddress::setMacArray(const u_int8_t arr[ETH_ALEN]) {
    copyFrom(arr);
}

void MacAddress::setArrayElement(size_t index, u_int8_t value) {
    mac.at(index) = value;
}

bool MacAddress::operator==(const MacAddress &other) const {
    bool equal = true;
    for (size_t i = 0; equal && (i < mac.size()); ++i) {
        equal &= mac[i] == other.mac[i];
    }
    return equal;
}

bool MacAddress::isUnset() const {
    Mac mac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    return *this == MacAddress(mac);
}

MacAddress MacAddress::GetBroadcastMac() {
    Mac destMac = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    return MacAddress(destMac);
}

ostream& Network::operator<<(ostream &strm, const MacAddress &mac) {
    char buff[18];
    const Mac& macAddr = mac.getMacArray();
    snprintf(buff, sizeof(buff), "%02x:%02x:%02x:%02x:%02x:%02x",
             macAddr[0],
             macAddr[1],
             macAddr[2],
             macAddr[3],
             macAddr[4],
             macAddr[5]
    );
    strm << buff;
    return strm;
}