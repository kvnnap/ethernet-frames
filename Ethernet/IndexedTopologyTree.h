//
// Created by kevin on 7/25/16.
//

#ifndef NETWORK_DISCOVERY_INDEXEDTOPOLOGYTREE_H
#define NETWORK_DISCOVERY_INDEXEDTOPOLOGYTREE_H

#include <ostream>
#include <vector>
#include <set>

namespace Network {

    class IndexedTopologyNode {
    public:
        IndexedTopologyNode(size_t p_val = 0);

        bool isLeaf() const;
        bool isRoot() const;
        void setParent(size_t parentIndex);
        void deleteChild(size_t childIndex);

        std::vector<std::size_t> children;
        std::size_t parent;
        std::size_t val;
        bool parentSet;

        // When in connection with first, violations in second apply
        std::vector<std::pair<std::size_t, std::set<std::size_t>>> violators;
        //std::set<std::size_t> childVals;
    };

    std::ostream& operator<< (std::ostream& strm, const IndexedTopologyNode& indexedTopologyNode);

    class IndexedTopologyTree {
    public:
        size_t getNewNode();
        size_t addNewNode(const IndexedTopologyNode& node);
        IndexedTopologyNode& getNode(size_t index);
        void addChildToParent(size_t childIndex, size_t parentIndex);
        void moveLeafToNode(size_t childIndex, size_t parentIndex);
        void recomputeNodeState(size_t nodeIndex);
        void clear();
        // A lhs val cannot be contained in the closest subtree containing rhsVal
        void addRule(size_t lhsVal, size_t rhsVal1, size_t rhsVal2);

        const IndexedTopologyNode& getNode(size_t index) const;
        const std::vector<IndexedTopologyNode>& getNodes() const;
        bool canValBePlaced(size_t valNodeIndex, size_t otherNodeIndex) const;

        size_t findClosestCommonSubtree(size_t nodeAIndex, size_t nodeBIndex) const;
        size_t findNode(size_t val) const;
        size_t findParentNodeOf(size_t val) const;
        bool contains(size_t subtreeIndex, size_t val) const;

    private:
        std::vector<IndexedTopologyNode> nodes;
    };

    std::ostream& operator<< (std::ostream& strm, const IndexedTopologyTree& indexedTopologyTree);
}


#endif //NETWORK_DISCOVERY_INDEXEDTOPOLOGYTREE_H
