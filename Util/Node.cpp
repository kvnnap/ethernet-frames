//
// Created by kevin on 9/15/16.
//

#include <sstream>
#include <fstream>

#include "Node.h"

using namespace std;
using namespace Util;

AbstractNode::AbstractNode()
        : parent (nullptr)
{

}
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

void AbstractNode::setParent(Node *p_parent) {
    parent = p_parent;
}

Node *AbstractNode::getParent() {
    return parent;
}

bool AbstractNode::deleteValue(const Network::MacAddress &value) {
    AbstractNode * nodeToDelete = findNode(value);
    if (nodeToDelete != nullptr) {
        Node * parent = nodeToDelete->getParent();
        if (parent == nullptr) {
            throw runtime_error ("Cannot delete node that has no parent");
        }
        // Find child in parent node
        if (!parent->deleteChild(nodeToDelete)) {
            throw runtime_error ("Child to delete cannot be found in the parent node. Tree is an inconsistent state");
        }
        return true;
    }
    return false;
}

NodeType Node::getType() const {
    return NODE;
}

const std::vector<NodePt> &Node::getChildren() const {
    return children;
}

void Node::addChild(NodePt child) {
    child->setParent(this);
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

Leaf *Node::findNode(const Network::MacAddress &value) {
    for (const NodePt& child : children) {
        Leaf * result = child->findNode(value);
        if (result != nullptr) {
            return result;
        }
    }
    return nullptr;
}

NodePt Node::deleteChild(const AbstractNode *node) {
    for (vector<NodePt>::iterator it = children.begin(); it != children.end(); ++it) {
        if ((*it).get() == node) {
            NodePt ret = move(*it);
            children.erase(it);
            return ret;
        }
    }
    return NodePt();
}

NodePt Node::makeRoot(NodePt currentRoot) {
    // Find path from this node to currentRoot
    vector<Node *> path;
    Node * node = this;
    do {
        // Add to path
        path.push_back(node);

        // Update Variable
        node = node->getParent();
    } while (node != nullptr);

    // Check
    if (currentRoot.get() != static_cast<AbstractNode*>(path[path.size() - 1])) {
        throw runtime_error("Root Mismatch: The currentRoot passed as parameter is not the actual currentRoot");
    }

    // Make modifications
    for (ssize_t i = path.size() - 1; i > 0; --i) {
        // Gather nodes
        Node * parent = path[i]; // currentRoot and parent are in sync
        Node * child = path[i - 1];

        // Make child the parent of parent
        NodePt childST = parent->deleteChild(child);
        childST->setParent(nullptr);
        static_cast<Node *>(childST.get())->addChild(move(currentRoot));

        //
        currentRoot = move(childST);
    }

    return currentRoot;
}

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
}

Leaf* Leaf::findNode(const Network::MacAddress &p_value) {
    if (value == p_value) {
        return this;
    }
    return nullptr;
};