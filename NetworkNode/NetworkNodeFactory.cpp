//
// Created by kevin on 7/24/16.
//

#include <vector>
#include <algorithm>
#include <sstream>

#include "XML/rapidxml_utils.hpp"
#include "Util/StringOperations.h"
#include "NetworkNodeFactory.h"

// Random
#include "Sampler/UniformSampler.h"

using namespace std;
using namespace rapidxml;

using namespace Network;


unique_ptr<NetworkNode> NetworkNodeFactory::make(const string & value) const {
    // Select between XML and Random and possible others - Format is <Type>:<Settings>
    vector<string> parameters = Util::StringOperations::split(value, ':');
    if (parameters.size() != 2) {
        throw invalid_argument("NetworkNodeFactory expecting two colon-separated values, but got: " + value);
    }

    // lower case
    transform(parameters[0].begin(), parameters[0].end(), parameters[0].begin(), ::tolower);

    // Dispatch to appropriate factory
    if (parameters[0] == "xml") {
        return XMLNetworkNodeFactory().make(parameters[1]);
    }  else if (parameters[0] == "random") {
        return RandomNetworkNodeFactory().make(parameters[1]);
    } else {
        throw invalid_argument("NetworkNodeFactory - Unknown type: " + parameters[0]);
    }
}

unique_ptr<NetworkNode> XMLNetworkNodeFactory::make(const string &fileName) const {

    // Open document
    unique_ptr<file<>> xmlFile(new file<>(fileName.c_str()));
    xml_document<> doc;
    doc.parse<0>(xmlFile->data());

    // Get Switch
    return getSwitch(doc.first_node());
}

unique_ptr<SwitchNode> XMLNetworkNodeFactory::getSwitch(xml_node<> *xmlSwitchNode) {
    unique_ptr<SwitchNode> switchNode (new SwitchNode());
    for (xml_node<> *child = xmlSwitchNode->first_node(); child; child = child->next_sibling()) {
        string childName (child->name());
        if (childName == "switch") {
            switchNode->add(getSwitch(child));
        } else if (childName == "device") {
            switchNode->add(getDevice(child));
        } else {
            throw runtime_error("Malformed XML File. Invalid XML Node: " + childName);
        }
    }
    return switchNode;
}

std::unique_ptr<NetDeviceNode> XMLNetworkNodeFactory::getDevice(xml_node<> *xmlDeviceNode) {
    string mac;
    for (xml_attribute<> *attr = xmlDeviceNode->first_attribute(); attr; attr = attr->next_attribute())
    {
        string attrName (attr->name());
        if (attrName == "mac") {
            mac = attr->value();
        } else {
            throw runtime_error("Malformed XML File. Invalid XML Attribute: " + attrName);
        }
    }
    return unique_ptr<NetDeviceNode>(new NetDeviceNode(mac));
}

std::unique_ptr<NetworkNode> RandomNetworkNodeFactory::make(const std::string &settings) const {
    // Input is number of vertices (nodes) - extract this
    size_t numVertices ( 0 );
    if (!(stringstream(settings) >> numVertices)) {
        throw invalid_argument("RandomNetworkNodeFactory - expecting number but got: " + settings);
    }

    // Produce a random Prüfer Sequence
    vector<size_t> pruferSequence = generateRandomPruferSequence(numVertices);

    // Generate tree given a Prüfer Sequence


    throw logic_error("RandomNetworkNodeFactory is not implemented");
}

vector<size_t> RandomNetworkNodeFactory::generateRandomPruferSequence(size_t numVertices) {
    vector<size_t> sequence;

    if (numVertices < 2) {
        throw runtime_error("generateRandomPruferSequence: numVertices should be >= 2. It is: " + to_string(numVertices));
    }

    sequence.reserve(numVertices - 2);

    Sampler::UniformSampler uniformSampler;
    for (size_t i = 0; i < numVertices - 2; ++i) {
        sequence.push_back(uniformSampler.nextSample(1, numVertices));
    }

    return sequence;
}

std::vector<size_t> RandomNetworkNodeFactory::generateTreeFromPuferSequence(const std::vector<size_t> &pruferSequence) {
    vector<size_t> parentTree (pruferSequence.size() + 2);
    vector<size_t> degree (pruferSequence.size() + 2, 1);

    // Initialise degrees
    for (size_t slot : pruferSequence) {
        ++degree[slot - 1];
    }

    // Insert edges by finding nodes of degree 1 (which are not in prufer sequence)
    for (size_t slot : pruferSequence) {
        for (size_t degIndex = 0; degIndex < degree.size(); ++degIndex) {
            if (degree[degIndex] == 1) {
                // Connect in tree node[degIndex] with node[slot - 1]
                // reduce degrees
                --degree[slot - 1];
                --degree[degIndex];
            }
        }
    }

    // Find last two nodes having degree value 1
    size_t u ( 0 );
    size_t v;
    for (size_t degIndex = 0; degIndex < degree.size(); ++degIndex) {
        if (degree[degIndex] == 1) {
            if (u == 0) {
                u = degIndex;
            } else {
                v = degIndex;
            }
        }
    }

    // Connect in tree, node[u] with node[v]

    return vector<size_t>();
}
