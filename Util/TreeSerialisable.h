//
// Created by kevin on 9/15/16.
//

#ifndef NETWORK_DISCOVERY_TREESERIALISER_H
#define NETWORK_DISCOVERY_TREESERIALISER_H

#include "Node.h"

namespace Util {
    class TreeSerialisable {
    public:
        virtual ~TreeSerialisable() {};
        virtual NodePt toTree() const = 0;
    };
}

#endif //NETWORK_DISCOVERY_TREESERIALISER_H
