//
// Created by kevin on 7/24/16.
//


#include "XML/rapidxml_utils.hpp"

#include "NetworkNodeFactory.h"

using namespace std;
using namespace rapidxml;

using namespace Network;

unique_ptr<NetworkNode> NetworkNodeFactory::make(const string &name) const {

    // Open document
    unique_ptr<file<>> xmlFile(new file<>(name.c_str()));
    xml_document<> doc;
    doc.parse<0>(xmlFile->data());

    // Get Switch
    return getSwitch(doc.first_node());
}

unique_ptr<SwitchNode> NetworkNodeFactory::getSwitch(xml_node<> *xmlSwitchNode) {
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

std::unique_ptr<NetDeviceNode> NetworkNodeFactory::getDevice(xml_node<> *xmlDeviceNode) {
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
