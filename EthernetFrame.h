//
// Created by kevin on 5/28/16.
//

#ifndef NETWORK_DISCOVERY_ETHERNETFRAME_H
#define NETWORK_DISCOVERY_ETHERNETFRAME_H

#include "MacAddress.h"

namespace Network {

#pragma pack(push, 1)
    class EthernetFrame {
    public:

        EthernetFrame();
        EthernetFrame(const u_int8_t * src);

        void copyTo(u_int8_t * dest) const;
        void copyFrom(const u_int8_t * src);
        void setEtherType(u_int16_t p_ethType);
        u_int16_t getEtherType() const;

        bool operator== (const EthernetFrame& other) const;

        MacAddress destinationMac;
        MacAddress sourceMac;
    private:
        // Stored in network byte order
        u_int16_t etherType;
    };

#pragma pack(pop)

    std::ostream& operator<< (std::ostream& strm, const EthernetFrame& ef);


}




#endif //NETWORK_DISCOVERY_ETHERNETFRAME_H
