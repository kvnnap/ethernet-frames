//
// Created by kevin on 7/25/16.
//

#include <stdexcept>
#include <algorithm>
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

bool IndexedTopologyNode::isRoot() const {
    return !parentSet;
}

void IndexedTopologyNode::deleteChild(size_t childIndex) {
    children.erase(remove(children.begin(), children.end(), childIndex), children.end());
}

ostream& Network::operator<<(ostream &strm, const IndexedTopologyNode &itn) {
    strm << "IndexedTopologyNode:" << endl
         << "\tisLeaf: " << (itn.isLeaf() ? "Yes" : "No" ) << endl;
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
    return nodes.at(index);
}

const IndexedTopologyNode& IndexedTopologyTree::getNode(size_t index) const {
    return nodes.at(index);
}

const std::vector<IndexedTopologyNode>& IndexedTopologyTree::getNodes() const {
    return nodes;
}

void IndexedTopologyTree::clear() {
    nodes.clear();
}

size_t IndexedTopologyTree::findNode(size_t val) const {
    for (size_t index = 0; index < nodes.size(); ++index) {
        const IndexedTopologyNode& node = getNode(index);
        if (node.isLeaf() && node.val == val) {
            return index;
        }
    }
    // not found
    throw runtime_error("Node not found. Was searching for: " + to_string(val));
}

size_t IndexedTopologyTree::findParentNodeOf(size_t val) const {
    size_t nodeIndex = findNode(val);
    if (nodes[nodeIndex].parentSet) {
        return nodes[nodeIndex].parent;
    }
    else {
        throw runtime_error("Node found but has no parent node");
    }
}

bool IndexedTopologyTree::contains(size_t subtreeIndex, size_t val) const {
    const IndexedTopologyNode& node = getNode(subtreeIndex);
    if (node.isLeaf() && val == node.val) {
        return true;
    } else {
        for (size_t childIndex : node.children) {
            if (contains(childIndex, val)) {
                return true;
            }
        }
        return false;
    }
}

void IndexedTopologyTree::recomputeNodeState(size_t nodeIndex) {
    // Recompute violators
    IndexedTopologyNode& node = getNode(nodeIndex);
    node.violators.clear();

    // Gather all children - recursively

    for (size_t childIndex : node.children) {
        const IndexedTopologyNode& childNode = getNode(childIndex);
        if (childNode.isLeaf()) {
            node.violators.insert(childNode.violators.begin(), childNode.violators.end());
        }
    }
}

void IndexedTopologyTree::moveLeafToNode(size_t childIndex, size_t parentIndex) {
    IndexedTopologyNode& childLeaf = getNode(childIndex);

    // Delete leaf from current parent and restore state, if any
    if (!childLeaf.isRoot()) {
        IndexedTopologyNode &currentParent = getNode(childLeaf.parent);
        currentParent.deleteChild(childIndex);
        recomputeNodeState(childLeaf.parent);
    }

    // Add
    IndexedTopologyNode& parentNode = getNode(parentIndex);
    parentNode.children.push_back(childIndex);
    childLeaf.setParent(parentIndex);
    recomputeNodeState(parentIndex);
}

void IndexedTopologyTree::addRule(size_t lhsNodeVal, size_t rhsNodeVal1, size_t rhsNodeVal2) {
    // lhsNode < rhsNode
    // The lhsNode can never be in the same subtree as the rhsNode

    // Find lhsNode and check if it lies in the same subtree as the rhsNode
    findClosestCommonSubtree(rhsNodeVal1, rhsNodeVal2);
    size_t rhsNodeIndex = findNode(rhsNodeVal);
    size_t rhsNodeParentIndex = findParentNodeOf(rhsNodeVal);

    // Add facts to node
    getNode(rhsNodeIndex).violators.insert(lhsNodeVal);

    // Optimisation
    getNode(rhsNodeParentIndex).violators.insert(lhsNodeVal);

    // -- Does LHS lie in RHS subtree?
    if (contains(rhsNodeParentIndex, lhsNodeVal)) {
        // Violation - LHS lies in RHS subtree and LHS NOT< RHS- Tree transformation needed

        // New method -
        // Check children nodes of rhs Parent
        bool moved = false;
        IndexedTopologyNode& rhsNodeParent = getNode(rhsNodeParentIndex);
        for (size_t childIndex : rhsNodeParent.children) {
            IndexedTopologyNode& child = getNode(childIndex);
            if (!child.isLeaf()) {
                // check if we can transfer rhsNode here
                if (canValBePlaced(rhsNodeIndex, childIndex)) {
                    moveLeafToNode(rhsNodeIndex, childIndex);
                    moved = true;
                    break;
                }
            }
        }

        if (!moved) {
            // Encapsulate rhsNode into a new node and move it there
            // Add new child to rhsNodeParent node
            size_t newNodeIndex = getNewNode();
            addChildToParent(newNodeIndex, rhsNodeParentIndex);
            moveLeafToNode(rhsNodeIndex, newNodeIndex);
        }

        // need to move lhs node out of rhs node subtree and place it either in:
        // 1) Grand parent of RHS node
        // 2) (other children of Grand parent) Uncle of RHS node
        // 3) Move up one level (grand grand..) and try there

    }

}

bool IndexedTopologyTree::canValBePlaced(size_t valNodeIndex, size_t otherNodeIndex) const {
    // Check that val is not in the violations list of the node
    const IndexedTopologyNode& otherNode = getNode(otherNodeIndex);
    const IndexedTopologyNode& valNode = getNode(valNodeIndex);
    if (otherNode.violators.find(valNode.val) != otherNode.violators.end()) {
        return false;
    }
    // Check whether there are any of valNode violators in the otherNode subtree
    for (size_t violator : valNode.violators) {
        if (contains(otherNodeIndex, violator)) {
            return false;
        }
    }
    return true;
}

size_t IndexedTopologyTree::findClosestCommonSubtree(size_t nodeAIndex, size_t nodeBIndex) const {
    size_t currNodeA = nodeAIndex;
    size_t currNodeB = nodeBIndex;

    while (currNodeA != currNodeB && getNode(currNodeA).parentSet) {
        currNodeA = getNode(currNodeA).parent;
        currNodeB = nodeBIndex;
        while (currNodeA != currNodeB && getNode(currNodeB).parentSet) {
            currNodeB = getNode(currNodeB).parent;
        }
    }

    return currNodeA;
}

ostream& Network::operator<<(ostream &strm, const IndexedTopologyTree &itt) {
    for (size_t i = 0; i < itt.getNodes().size(); ++i) {
        strm << i << ") " << itt.getNodes()[i] << endl;
    }
    return strm;
}
