//
// Created by kevin on 8/5/16.
//

#ifndef NETWORK_DISCOVERY_DATAENCODER_H
#define NETWORK_DISCOVERY_DATAENCODER_H

#include <exception>
#include <arpa/inet.h>

namespace Network {

    class DataEncoder {

    public:
        // Statics
        // primitive to network byte order
        template <class T>
        static void writeValToNetworkBytes(T val, uint8_t *buff) {
            uint8_t * pVal = (uint8_t *)&val;
            // copying avoids type punning and guarantees correctness on all compilers (chars can alias)
            switch (sizeof(val)) {
                case 1:
                    *buff = *pVal;
                    break;
                case 2:
                {
                    uint16_t sVal = (pVal[0] << 8) | (pVal[1] << 0);
                    uint16_t nsVal = htons(sVal);
                    memcpy(buff, &nsVal, sizeof(nsVal));
                }
                    break;
                case 4: {
                    uint32_t iVal = (pVal[0] << 24) | (pVal[1] << 16) | (pVal[2] << 8) | (pVal[3] << 0);
                    uint32_t niVal = htonl(iVal);
                    memcpy(buff, &niVal, sizeof(niVal));
                }
                    break;
                default:
                    throw std::runtime_error("writeValToNetworkBytes: Unknown size");
            }
        }

        // Network to Primitive byte order
        template <class T>
        static T readValFromNetworkBytes(uint8_t * buff) {
            T val;
            // Chars and friends can alias
            uint8_t * pVal = (uint8_t *)&val;
            switch (sizeof(T)) {
                case 1:
                    return static_cast<T>(*buff);
                case 2:
                {
                    uint16_t hVal = ntohs((buff[0] << 8) | (buff[1] << 0));
                    uint8_t * phVal = (uint8_t *)&hVal;
                    memcpy(pVal, phVal, sizeof(hVal));
                    return val;
                }
                case 4:
                {
                    uint32_t hVal = ntohl((buff[0] << 24) | (buff[1] << 16) | (buff[2] << 8) | (buff[3] << 0));
                    uint8_t * phVal = (uint8_t *)&hVal;
                    memcpy(pVal, phVal, sizeof(hVal));
                    return val;
                }
                default:
                    throw std::runtime_error("readValFromNetworkBytes: Unknown size");
            }
        }
    };
}

#endif //NETWORK_DISCOVERY_DATAENCODER_H
