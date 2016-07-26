//
// Created by kevin on 7/25/16.
//

#ifndef NETWORK_DISCOVERY_INDEXEDTOPOLOGYTREE_H
#define NETWORK_DISCOVERY_INDEXEDTOPOLOGYTREE_H

#include <ostream>
#include <vector>

namespace Network {

    class IndexedTopologyNode {
    public:
        IndexedTopologyNode(size_t p_val = 0);

        bool isLeaf() const;
        void setParent(size_t parentIndex);

        std::vector<std::size_t> children;
        std::size_t parent;
        std::size_t val;
        bool parentSet;
    };

    std::ostream& operator<< (std::ostream& strm, const IndexedTopologyNode& indexedTopologyNode);

    class IndexedTopologyTree {
    public:
        size_t getNewNode();
        size_t addNewNode(const IndexedTopologyNode& node);

        IndexedTopologyNode& getNode(size_t index);
        const std::vector<IndexedTopologyNode>& getNodes() const;

        void addChildToParent(size_t childIndex, size_t parentIndex);
    private:
        std::vector<IndexedTopologyNode> nodes;
    };

    std::ostream& operator<< (std::ostream& strm, const IndexedTopologyTree& indexedTopologyTree);
}


#endif //NETWORK_DISCOVERY_INDEXEDTOPOLOGYTREE_H
