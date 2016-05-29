//
// Created by kevin on 5/27/16.
//

#ifndef NETWORK_DISCOVERY_MACADDRESS_H
#define NETWORK_DISCOVERY_MACADDRESS_H

#include <net/ethernet.h>
#include <cstdint>
#include <array>
#include <ostream>

namespace Network {

    typedef std::array<u_int8_t, ETH_ALEN> Mac;

#pragma pack(push, 1)
    class MacAddress {
    public:

        MacAddress();
        MacAddress(const Mac& p_mac);

        bool operator==(const MacAddress& other) const;
        bool operator!=(const MacAddress& other) const;
        bool isUnset() const;

        const Mac& getMacArray() const;
        void copyTo(u_int8_t * dest) const;
        void copyFrom(const u_int8_t * src);

        void setMacArray(const Mac& p_mac);
        void setMacArray(const u_int8_t arr[ETH_ALEN]);
        void setArrayElement(size_t index, u_int8_t value);

        static MacAddress GetBroadcastMac();

        //friend std::ostream& operator<< (std::ostream& strm, const MacAddress& mac);
    private:
        Mac mac;
    };
#pragma pack(pop)

    std::ostream& operator<< (std::ostream& strm, const MacAddress& mac);

}


#endif //NETWORK_DISCOVERY_MACADDRESS_H
