//
// Created by kevin on 7/25/16.
//

#include "IndexedTopologyTree.h"

using namespace std;
using namespace Network;


bool IndexedTopologyNode::isLeaf() const {
    return children.empty();
}

IndexedTopologyNode::IndexedTopologyNode()
    : parent(), val (), parentSet ()
{
}

IndexedTopologyNode::IndexedTopologyNode(size_t parentIndex, size_t p_val)
    : parent(parentIndex), val (p_val), parentSet (true)
{
}

ostream& Network::operator<<(ostream &strm, const IndexedTopologyNode &itn) {
    strm << "IndexedTopologyNode:" << endl
         << "\tisLeaf: " << itn.isLeaf() << endl
         << "\tparent: set: " << itn.parentSet;
         if (itn.parentSet ) {
             strm << ", parentVal: " << itn.parent << endl;
         } else {
             strm << endl;
         }
         strm << "\tval: " << itn.val << endl
         << "\tchildren: {";
    for (size_t setId : itn.children) {
        strm << setId;
        if (setId != *--itn.children.end()) {
            strm << ", ";
        }
    }
    strm << "}" << endl;
    return strm;
}

void IndexedTopologyTree::addChildToParent(size_t childIndex, size_t parentIndex) {
    nodes[parentIndex].children.push_back(childIndex);
    nodes[childIndex].parent = parentIndex;
    nodes[childIndex].parentSet = true;
}

size_t IndexedTopologyTree::getNewNode() {
    nodes.push_back(IndexedTopologyNode());
    return nodes.size() - 1;
}

size_t IndexedTopologyTree::addNewNode(const IndexedTopologyNode &node) {
    nodes.push_back(node);
    return nodes.size() - 1;
}





