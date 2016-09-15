//
// Created by kevin on 9/15/16.
//

#include "Node.h"

using namespace Util;

INode::~INode() {

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

bool Node::operator==(const INode &other) const {
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
//bool Leaf<T>::operator==(const INode &other) const {
//    return other.getType() == LEAF && value == static_cast<const Leaf&>(other).value;
//}

//template class Leaf<MacAddress>;