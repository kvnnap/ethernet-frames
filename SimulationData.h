//
// Created by kevin on 6/20/16.
//

#ifndef NETWORK_DISCOVERY_SIMULATIONDATA_H
#define NETWORK_DISCOVERY_SIMULATIONDATA_H

#include "NetworkNode.h"

namespace Network {
    class SimulationData {
    public:
        NetworkNode * to;
        NetworkNode * from;
        const uint8_t * buffer;
        size_t lenBuffer;
    };
}

#endif //NETWORK_DISCOVERY_SIMULATIONDATA_H
