//
// Created by kevin on 5/29/16.
//

#ifndef NETWORK_DISCOVERY_ETHERNETDISCOVERY_H
#define NETWORK_DISCOVERY_ETHERNETDISCOVERY_H

#include "EthernetSocket.h"

#include "ISocketListener.h"

namespace  Network {

    class EthernetDiscovery
        : public ISocketListener
    {
    public:

        enum MessageType : uint8_t
        {
            EMPTY = 0,
            HELLO = 1,
            BEGIN = 2,
            READY = 3,
            START = 4,
            TEST = 5,
            REQUEST = 6,
            YES = 7,
            NO = 8
        };

        EthernetDiscovery(EthernetSocket& ethernetSocket);

        bool dataArrival(EthernetFrame& ef, uint8_t * data, size_t len) override;
        void getAllDevices();
        void slave();

    private:
        EthernetSocket & ethernetSocket;

        std::vector<MacAddress> slaveMacs;

    };

}

#endif //NETWORK_DISCOVERY_ETHERNETDISCOVERY_H
