//
// Created by kevin on 7/24/16.
//

#ifndef NETWORK_DISCOVERY_NETWORKNODEFACTORY_H
#define NETWORK_DISCOVERY_NETWORKNODEFACTORY_H

#include "XML/rapidxml.hpp"

#include "Factory/IFactory.h"
#include "NetworkNode.h"
#include "SwitchNode.h"
#include "NetDeviceNode.h"

namespace Network {

    class NetworkNodeFactory
        : public IFactory<NetworkNode>
    {
    public:
        std::unique_ptr<NetworkNode> make(const std::string& value) const override;
    };

    class XMLNetworkNodeFactory
        : public IFactory<NetworkNode>
    {
    public:
        std::unique_ptr<NetworkNode> make(const std::string& fileName) const override;
    private:
        static std::unique_ptr<SwitchNode> getSwitch(rapidxml::xml_node<>* switchNode);
        static std::unique_ptr<NetDeviceNode> getDevice(rapidxml::xml_node<>* deviceNode);
    };

    class RandomNetworkNodeFactory
            : public IFactory<NetworkNode>
    {
    public:
        std::unique_ptr<NetworkNode> make(const std::string& settings) const override;

    private:
        static std::vector<size_t> generateRandomPruferSequence(size_t numVertices);
        static std::vector<size_t> generateTreeFromPuferSequence(const std::vector<size_t>& pruferSequence);
    };
}


#endif //NETWORK_DISCOVERY_NETWORKNODEFACTORY_H
