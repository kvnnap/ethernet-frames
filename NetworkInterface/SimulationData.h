//
// Created by kevin on 6/20/16.
//

#ifndef NETWORK_DISCOVERY_SIMULATIONDATA_H
#define NETWORK_DISCOVERY_SIMULATIONDATA_H

#include <cstdint>
#include <cstddef>

namespace Network {

    class NetworkNode;

    class SimulationData {
    public:
        SimulationData();

        NetworkNode * to;
        NetworkNode * from;
        const uint8_t * buffer;
        size_t lenBuffer;
        uint16_t  hops;
    };
}

#endif //NETWORK_DISCOVERY_SIMULATIONDATA_H
