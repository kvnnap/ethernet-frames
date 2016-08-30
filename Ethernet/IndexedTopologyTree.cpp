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

    // Gather all children
    vector<set<size_t>> childSets;
    for (size_t childIndex : node.children) {
        childSets.push_back(getChildrenOf(childIndex));
    }

    // loop and add to violators
    for (size_t s = 0; s < childSets.size(); ++s) {
        for (size_t sChild : childSets[s]) {
            for (size_t s2 = s; s2 < childSets.size(); ++s2) {
                for (size_t sChild2 : childSets[s2]) {
                    // Add violators here
                    map<set<size_t>, set<size_t>>::const_iterator itemIterator = factList.find({sChild, sChild2});
                    if (itemIterator != factList.end()) {
                        node.violators.insert(itemIterator->second.begin(), itemIterator->second.end());
                    }
                }
            }
        }
    }
}

void IndexedTopologyTree::moveNodeToNode(size_t nodeIndex, size_t parentIndex) {
    IndexedTopologyNode& node = getNode(nodeIndex);

    // Delete leaf from current parent and restore state, if any
    if (!node.isRoot()) {
        IndexedTopologyNode &currentParent = getNode(node.parent);
        currentParent.deleteChild(nodeIndex);
        recomputeNodeState(node.parent);
    }

    // Add
    IndexedTopologyNode& parentNode = getNode(parentIndex);
    parentNode.children.push_back(nodeIndex);
    node.setParent(parentIndex);
    recomputeNodeState(parentIndex);
}

void IndexedTopologyTree::addRule(size_t lhsNodeVal, size_t rhsNodeVal1, size_t rhsNodeVal2) {
    // lhsNode < rhsNode

    // Push fact so that we can compute validity later
    factList[{rhsNodeVal1, rhsNodeVal2}].insert(lhsNodeVal);

    // Find the closest common ancestor of rhsNodeVal1 and rhsNodeVal2
    size_t rhsNodeIndex = findClosestCommonAncestor(findNode(rhsNodeVal1), findNode(rhsNodeVal2));

    // Add facts to node
    getNode(rhsNodeIndex).violators.insert(lhsNodeVal);

    // -- Does LHS lie in RHS subtree?
    if (contains(rhsNodeIndex, lhsNodeVal)) {
        // Violation - LHS lies in RHS subtree and LHS NOT< RHS- Tree transformation needed

        // Get relevant subtree indices
        size_t lhsSubtreeIndex = getSubtreeContainingVal(rhsNodeIndex, lhsNodeVal);
        size_t rhsNodeVal1SubtreeIndex = getSubtreeContainingVal(rhsNodeIndex, rhsNodeVal1);
        size_t rhsNodeVal2SubtreeIndex = getSubtreeContainingVal(rhsNodeIndex, rhsNodeVal2);

        // Join rhsNodeVal1 and rhsNodeVal2 subtrees together, this step is always required

        if (canNodeBePlaced(rhsNodeVal1SubtreeIndex, rhsNodeVal2SubtreeIndex)) {
            // Case 1 - Join rhsNodeVal1SubtreeIndex to rhsNodeVal2SubtreeIndex
            moveNodeToNode(rhsNodeVal1SubtreeIndex, rhsNodeVal2SubtreeIndex);
        } else {
            if (canNodeBePlaced(rhsNodeVal2SubtreeIndex, rhsNodeVal1SubtreeIndex)) {
                // Case 2 - Join rhsNodeVal2SubtreeIndex to rhsNodeVal1SubtreeIndex
                moveNodeToNode(rhsNodeVal2SubtreeIndex, rhsNodeVal1SubtreeIndex);
            } else {
                // Case 3 - Join rhsNodeVal1SubtreeIndex and rhsNodeVal2SubtreeIndex to sibling nodes
                IndexedTopologyNode rhsNode = getNode(rhsNodeIndex);

                bool moved = false;
                for (size_t siblingIndex : rhsNode.children) {
                    if (siblingIndex != rhsNodeVal1SubtreeIndex &&
                        siblingIndex != rhsNodeVal2SubtreeIndex &&
                        siblingIndex != lhsSubtreeIndex &&
                        canNodeBePlaced(rhsNodeVal1SubtreeIndex, siblingIndex) &&
                        canNodeBePlaced(rhsNodeVal2SubtreeIndex, siblingIndex))
                    {
                        moveNodeToNode(rhsNodeVal1SubtreeIndex, siblingIndex);
                        moveNodeToNode(rhsNodeVal2SubtreeIndex, siblingIndex);
                        moved = true;
                        break;
                    }
                }

                // Case 4 - Join rhsNodeVal1SubtreeIndex and rhsNodeVal2SubtreeIndex to new Node
                if (!moved) {
                    size_t newNodeIndex = getNewNode();
                    moveNodeToNode(rhsNodeVal1SubtreeIndex, newNodeIndex);
                    moveNodeToNode(rhsNodeVal2SubtreeIndex, newNodeIndex);
                    moveNodeToNode(newNodeIndex, rhsNodeIndex);
                }
            }
        }

        // As a result, lhsNodeVal is now allowed in the rhsNode tree. However, one last case remains

        // Check whether LHS Value lies in child subtrees containing rhsNodeVal1 or rhsNodeVal2
        if (lhsSubtreeIndex == rhsNodeVal1SubtreeIndex || lhsSubtreeIndex == rhsNodeVal2SubtreeIndex) {
            // LHS Value lies in one of RHS Val 1/2 subtrees, this additional operation is needed
            throw runtime_error("This special case was not implemented");
        }

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

size_t IndexedTopologyTree::findClosestCommonAncestor(size_t nodeAIndex, size_t nodeBIndex) const {
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

size_t IndexedTopologyTree::getSubtreeContainingVal(size_t rootNodeIndex, size_t val) const {
    size_t valNodeIndex = findNode(val);
    size_t subtreeNodeIndex = valNodeIndex;

    while (getNode(subtreeNodeIndex).parentSet && getNode(subtreeNodeIndex).parent != rootNodeIndex) {
        subtreeNodeIndex = getNode(subtreeNodeIndex).parent;
    }

    if (!getNode(subtreeNodeIndex).parentSet) {
        throw runtime_error("Val is not contained in this subtree");
    }

    return subtreeNodeIndex;
}

std::set<size_t> IndexedTopologyTree::getChildrenOf(size_t nodeIndex) const {
    // TODO: Extremely Inefficient - Fix later
    set<size_t> vals;
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (nodes[i].isLeaf() && contains(nodeIndex, nodes[i].val)) {
            vals.insert(nodes[i].val);
        }
    }
    return vals;
}

// ATTENTION: This can only be used to when moving a node within the same subtree
// I.E siblings or lower - This is so that fewer checks are made
bool IndexedTopologyTree::canNodeBePlaced(size_t thisNodeIndex, size_t otherNodeIndex) const {
    const IndexedTopologyNode otherNode = getNode(otherNodeIndex);
    if (otherNode.isLeaf() || thisNodeIndex == otherNodeIndex) {
        return false;
    }
    set<size_t> thisNodeChildren = getChildrenOf(thisNodeIndex);
    vector<size_t> violations (thisNodeChildren.size());
    violations.resize(set_intersection(thisNodeChildren.begin(), thisNodeChildren.end(),
                                       otherNode.violators.begin(), otherNode.violators.end(),
                                   violations.begin())
                  - violations.begin());
    return violations.size() == 0;
}

ostream& Network::operator<<(ostream &strm, const IndexedTopologyTree &itt) {
    for (size_t i = 0; i < itt.getNodes().size(); ++i) {
        strm << i << ") " << itt.getNodes()[i] << endl;
    }
    return strm;
}
