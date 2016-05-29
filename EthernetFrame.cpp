//
// Created by kevin on 5/28/16.
//

#include <arpa/inet.h>

#include "EthernetFrame.h"

using namespace std;
using namespace Network;

EthernetFrame::EthernetFrame()
    : etherType (ETH_P_ALL)
{
}

EthernetFrame::EthernetFrame(const u_int8_t *src) {
    copyFrom(src);
}

void EthernetFrame::copyTo(u_int8_t *dest) const {
    destinationMac.copyTo(dest);
    dest += ETH_ALEN;

    sourceMac.copyTo(dest);
    dest += ETH_ALEN;

    *(u_int16_t*) dest  = etherType;
}

void EthernetFrame::copyFrom(const u_int8_t *src) {
    destinationMac.copyFrom(src);
    src += ETH_ALEN;

    sourceMac.copyFrom(src);
    src += ETH_ALEN;

    etherType = *(u_int16_t*) src;
}

bool EthernetFrame::operator==(const EthernetFrame &other) const {

    return destinationMac == other.destinationMac && sourceMac == other.sourceMac && etherType == other.etherType;
}

void EthernetFrame::setEtherType(u_int16_t p_ethType) {
    etherType = htons(p_ethType);
}

u_int16_t EthernetFrame::getEtherType() const {
    return ntohs(etherType);
}


ostream& Network::operator<<(ostream &strm, const EthernetFrame &ef) {
    return strm << ef.destinationMac << endl << ef.sourceMac << endl << ef.getEtherType();
}

