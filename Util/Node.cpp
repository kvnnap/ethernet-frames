//
// Created by kevin on 9/15/16.
//

#include <sstream>
#include <fstream>

#include "Node.h"

using namespace std;
using namespace Util;

AbstractNode::~AbstractNode() {

}

std::string AbstractNode::toDot() const {
    stringstream ss;
    size_t labelNum = 0;
    ss << "graph unnamed { S_0 [color=red, fontcolor=red]; " << endl << toDotEdges(labelNum) << "}";
    return ss.str();
}

void AbstractNode::toDotFile(const std::string &fileName) const {
    ofstream (fileName) << toDot() << endl;
}

NodeType Node::getType() const {
    return NODE;
}

const std::vector<NodePt> &Node::getChildren() const {
    return children;
}

void Node::addChild(NodePt child) {
    children.push_back(move(child));
}

bool Node::operator==(const AbstractNode &other) const {
    if (this == &other) {
        return true;
    }
    if (other.getType() != NODE) {
        return false;
    }
    const Node& otherNode = static_cast<const Node&>(other);
    if (getChildrenSize() != otherNode.getChildrenSize()) {
        return false;
    }
    for (size_t i = 0; i < getChildrenSize(); ++i) {
        if (!(*getChildren()[i] == *otherNode.getChildren()[i])) {
            return false;
        }
    }
    return true;
}

size_t Node::getChildrenSize() const {
    return children.size();
}

string Node::toDotEdges(size_t& labelNum) const {

    // This case should never happen!
    if (children.empty()) {
        return string();
    }

    //
    size_t myLabel = labelNum++;
    stringstream ss;
    for (const NodePt& nodePt : children) {
        // Delegate increasing label to nodes only
        string childString = nodePt->toDotEdges(labelNum);
        if (!childString.empty()) {
            ss << "S_" << myLabel << " -- " << childString;
        }
    }

    return ss.str();
}

//template <class T>
//NodeType Leaf<T>::getType() const {
//    return LEAF;
//}
//
//template <class T>
//T Leaf<T>::getValue() const {
//    return value;
//}
//
//template <class T>
//T &Leaf<T>::getValueRef() const {
//    return value;
//}
//
//template <class T>
//bool Leaf<T>::operator==(const AbstractNode &other) const {
//    return other.getType() == LEAF && value == static_cast<const Leaf&>(other).value;
//}

//template class Leaf<MacAddress>;

Leaf::Leaf(const Network::MacAddress &value)  : value ( value ) {

}

bool Leaf::operator==(const AbstractNode &other) const {
    return other.getType() == LEAF && value == static_cast<const Leaf&>(other).value;
}

std::string Leaf::toDotEdges(size_t &labelNum) const {
    std::stringstream ss;
    ss << '\"' << value << "\";" << std::endl;
    return ss.str();
}

NodeType Leaf::getType() const {
    return LEAF;
}

Network::MacAddress Leaf::getValue() const  {
    return value;
}

const Network::MacAddress &Leaf::getValueRef() const  {
    return value;
};