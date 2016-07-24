//
// Created by kevin on 5/29/16.
//

#ifndef NETWORK_DISCOVERY_ISOCKETLISTENER_H
#define NETWORK_DISCOVERY_ISOCKETLISTENER_H

#include "EthernetFrame.h"

namespace Network {
    class ISocketListener
    {
    public:
        virtual ~ISocketListener();

        // Events gathered from underlying socket
        /**
         * This is called from a separate thread
         */
        virtual bool dataArrival(EthernetFrame& ef, uint8_t * data, size_t len) = 0;
    };
}

#endif //NETWORK_DISCOVERY_ISOCKETLISTENER_H
