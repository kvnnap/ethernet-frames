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

unique_ptr<NetDeviceNode> XMLNetworkNodeFactory::getDevice(xml_node<> *xmlDeviceNode) {
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

    if (numVertices < 3) {
        throw invalid_argument("RandomNetworkNodeFactory - expecting number >= 3, but got: " + settings);
    }

    // Produce a random Prüfer Sequence
    vector<size_t> pruferSequence = generateRandomPruferSequence(numVertices);

    // Generate edges given a Prüfer Sequence
    vector<pair<size_t, size_t>> edges = generateEdgesFromPuferSequence(pruferSequence);

    // Generate tree given edges
    vector<unique_ptr<NetworkNode>> nodes (pruferSequence.size() + 2);

    // Add Switches
    for (size_t slot : pruferSequence) {
        unique_ptr<NetworkNode>& ptNode = nodes.at(slot);
        if (!ptNode) {
            ptNode.reset(new SwitchNode());
        }
    }

    // Add NetDeviceNodes
    for (size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex) {
        unique_ptr<NetworkNode>& ptNode = nodes[nodeIndex];
        MacAddress mac;
        // TODO: May overflow
        mac.setArrayElement(5, static_cast<uint8_t>(nodeIndex + 1));
        if (!ptNode) {
            ptNode.reset(new NetDeviceNode(mac));
        }
    }

    // All edges were acquired, form tree in correct directed order with respect to chosen root
    return getNode(nodes, edges, pruferSequence.at(0));
}

vector<size_t> RandomNetworkNodeFactory::generateRandomPruferSequence(size_t numVertices) {
    vector<size_t> sequence;

    if (numVertices < 2) {
        throw runtime_error("generateRandomPruferSequence: numVertices should be >= 2. It is: " + to_string(numVertices));
    }

    sequence.reserve(numVertices - 2);

    Sampler::UniformSampler uniformSampler;
    for (size_t i = 0; i < numVertices - 2; ++i) {
        sequence.push_back(uniformSampler.nextSample(0, numVertices - 1));
    }

    return sequence;
}

vector<pair<size_t, size_t>> RandomNetworkNodeFactory::generateEdgesFromPuferSequence(const std::vector<size_t> &pruferSequence) {
    // Edges
    vector<pair<size_t, size_t>> edges;

    // Initialise degrees
    vector<size_t> degree (pruferSequence.size() + 2, 1);
    for (size_t slot : pruferSequence) {
        ++degree.at(slot);
    }

    // Insert edges by finding nodes of degree 1 (which are not in prufer sequence)
    for (size_t slot : pruferSequence) {
        for (size_t degIndex = 0; degIndex < degree.size(); ++degIndex) {
            if (degree[degIndex] == 1) {
                // Connect in tree node[degIndex] with node[slot]
                edges.push_back({slot, degIndex});
                // reduce degrees
                --degree.at(slot);
                --degree[degIndex];
                // next slot
                break;
            }
        }
    }

    // Find last two nodes having degree value 1
    size_t u, v;
    bool uNotSet ( true );
    bool vNotSet ( true );
    for (size_t degIndex = 0; degIndex < degree.size(); ++degIndex) {
        if (degree[degIndex] == 1) {
            if (uNotSet) {
                u = degIndex;
                uNotSet = false;
            } else if (vNotSet) /*sanity check*/{
                v = degIndex;
                vNotSet = false; // do not break for sanity check
            } else {
                throw runtime_error("RandomNetworkNodeFactory::generateTreeFromPuferSequence: More than two nodes with degree 1");
            }
        }
    }

    // Another sanity check
    if (uNotSet || vNotSet) {
        throw runtime_error("RandomNetworkNodeFactory::generateTreeFromPuferSequence: Did not finish with two nodes of degree 1");
    }

    // Connect in tree, node[u] with node[v]
    edges.push_back({u, v});

    return edges;
}

std::unique_ptr<NetworkNode> RandomNetworkNodeFactory::getNode(std::vector<std::unique_ptr<NetworkNode>> &nodes,
                                                               std::vector<std::pair<size_t, size_t>> &edges, size_t currentNode) {
    // Gather the child edges and delete them
    vector<size_t> childEdges;
    for (vector<pair<size_t, size_t>>::iterator it = edges.begin(); it != edges.end();) {
        if (currentNode == it->first || currentNode == it->second) {
            size_t childToAdd = currentNode == it->first ? it->second : it->first;
            childEdges.push_back(childToAdd);
            it = edges.erase(it);
        } else {
            ++it;
        }
    }

    // Move child nodes and add them to this node
    for (size_t childEdge : childEdges) {
        nodes.at(currentNode)->add(getNode(nodes, edges, childEdge));
    }

    return move(nodes.at(currentNode));
}
