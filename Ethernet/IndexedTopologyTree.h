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
        IndexedTopologyNode();
        IndexedTopologyNode(size_t parentIndex, size_t p_val = 0);

        bool isLeaf() const;

        std::vector<std::size_t> children;
        std::size_t parent;
        std::size_t val;
        bool parentSet;
    };

    std::ostream& operator<< (std::ostream& strm, const IndexedTopologyNode& mac);

    class IndexedTopologyTree {
    public:
        size_t getNewNode();
        size_t addNewNode(const IndexedTopologyNode& node);

        void addChildToParent(size_t childIndex, size_t parentIndex);
    private:
        std::vector<IndexedTopologyNode> nodes;
    };
}


#endif //NETWORK_DISCOVERY_INDEXEDTOPOLOGYTREE_H
