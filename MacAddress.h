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

    // Preferred way to typedef
    using Mac = std::array<uint8_t, ETH_ALEN>;

#pragma pack(push, 1)
    class MacAddress {
    public:

        static const MacAddress BroadcastMac;
        static const MacAddress& GetBroadcastMac();
        static const MacAddress UA1;
        static const MacAddress UA2;

        MacAddress();
        MacAddress(const Mac& p_mac);

        bool operator==(const MacAddress& other) const;
        bool operator!=(const MacAddress& other) const;
        bool isUnset() const;

        const Mac& getMacArray() const;
        void copyTo(uint8_t * dest) const;
        void copyFrom(const uint8_t * src);

        void setMacArray(const Mac& p_mac);
        void setMacArray(const uint8_t arr[ETH_ALEN]);
        void setArrayElement(size_t index, uint8_t value);


        //friend std::ostream& operator<< (std::ostream& strm, const MacAddress& mac);
    private:
        Mac mac;
    };
#pragma pack(pop)

    std::ostream& operator<< (std::ostream& strm, const MacAddress& mac);

}


#endif //NETWORK_DISCOVERY_MACADDRESS_H
