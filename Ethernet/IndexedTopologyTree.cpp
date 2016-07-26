//
// Created by kevin on 7/25/16.
//

#include "IndexedTopologyTree.h"

using namespace std;
using namespace Network;


bool IndexedTopologyNode::isLeaf() const {
    return children.empty();
}

IndexedTopologyNode::IndexedTopologyNode(size_t p_val)
    : parent(), val (p_val), parentSet ()
{
}

void IndexedTopologyNode::setParent(size_t parentIndex) {
    parent = parentIndex;
    parentSet = true;
}

ostream& Network::operator<<(ostream &strm, const IndexedTopologyNode &itn) {
    strm << "IndexedTopologyNode:" << endl
         << "\tisLeaf: " << itn.isLeaf() << endl;
         if (itn.parentSet) {
             strm << "\tparent: " << itn.parent << endl;
         } else {
             strm << "\tparent: N/A (Is Root)" << endl;
         }
         strm << "\tval: " << itn.val << endl
         << "\tchildren: {";
    for (size_t setId : itn.children) {
        strm << setId;
        if (setId != *--itn.children.end()) {
            strm << ", ";
        }
    }
    strm << "}";
    return strm;
}

void IndexedTopologyTree::addChildToParent(size_t childIndex, size_t parentIndex) {
    nodes[parentIndex].children.push_back(childIndex);
    nodes[childIndex].setParent(parentIndex);
}

size_t IndexedTopologyTree::getNewNode() {
    nodes.push_back(IndexedTopologyNode());
    return nodes.size() - 1;
}

size_t IndexedTopologyTree::addNewNode(const IndexedTopologyNode &node) {
    nodes.push_back(node);
    return nodes.size() - 1;
}

IndexedTopologyNode& IndexedTopologyTree::getNode(size_t index) {
    return nodes[index];
}

const std::vector<IndexedTopologyNode>& IndexedTopologyTree::getNodes() const {
    return nodes;
}

ostream& Network::operator<<(ostream &strm, const IndexedTopologyTree &itt) {
    for (size_t i = 0; i < itt.getNodes().size(); ++i) {
        strm << i << ") " << itt.getNodes()[i] << endl;
    }
    return strm;
}
