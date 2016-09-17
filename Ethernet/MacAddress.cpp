//
// Created by kevin on 5/27/16.
//

#include "MacAddress.h"

using namespace std;
using namespace Network;

const MacAddress MacAddress::BroadcastMac ((Mac){0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
const MacAddress MacAddress::UA1 ((Mac){0x00, 0x00, 0x5E, 0x00, 0x52, 0x02});
const MacAddress MacAddress::UA2 ((Mac){0x00, 0x00, 0x5E, 0x00, 0x52, 0x03});

MacAddress::MacAddress()
    : mac ()
{ }

MacAddress::MacAddress(const Mac &p_mac)
    : mac (p_mac)
{ }

MacAddress::MacAddress(const std::string &str) {
    sscanf(str.c_str(), "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
           &mac[0],
           &mac[1],
           &mac[2],
           &mac[3],
           &mac[4],
           &mac[5]
    );
}

const Mac& MacAddress::getMacArray() const {
    return mac;
};

void MacAddress::copyTo(uint8_t *dest) const {
    copy(mac.data(), mac.data() + mac.size(), dest);
}

void MacAddress::copyFrom(const uint8_t *src) {
    copy(src, src + mac.size(), mac.data());
}

void MacAddress::setMacArray(const Mac &p_mac) {
    mac = p_mac;
}

void MacAddress::setMacArray(const uint8_t arr[ETH_ALEN]) {
    copyFrom(arr);
}

void MacAddress::setArrayElement(size_t index, uint8_t value) {
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

const MacAddress& MacAddress::GetBroadcastMac() {
    return BroadcastMac;
}

bool MacAddress::operator!=(const MacAddress &other) const {
    return !(*this == other);
}

bool MacAddress::operator<(const MacAddress &b) const {
    for (size_t i = 0; i < getMacArray().size(); ++i) {
        if (getMacArray().at(i) != b.getMacArray().at(i)) {
            return getMacArray().at(i) < b.getMacArray().at(i);
        }
    }
    return false;
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