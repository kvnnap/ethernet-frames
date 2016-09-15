//
// Created by kevin on 9/15/16.
//

#ifndef NETWORK_DISCOVERY_NODE_H
#define NETWORK_DISCOVERY_NODE_H

#include <vector>
#include <memory>

namespace Util {
    enum NodeType {
        NODE,
        LEAF
    };

    class INode {
    public:

        virtual ~INode();

        virtual NodeType getType() const = 0;
        virtual bool operator== (const INode& other) const = 0;
    };

    using NodePt = std::unique_ptr<INode>;
}
namespace Util {
    class Node
        : public INode
    {
    public:

        NodeType getType() const override;
        virtual bool operator== (const INode& other) const override;

        const std::vector<NodePt>& getChildren() const;
        size_t getChildrenSize() const;
        void addChild(NodePt child);

    private:
        std::vector<NodePt> children;
    };
}

namespace Util {
    template <class T>
    class Leaf
        : public INode
    {
    public:

        Leaf(const T& value) : value ( value ) {};
        NodeType getType() const override {
            return LEAF;
        }
        virtual bool operator== (const INode& other) const override {
            return other.getType() == LEAF && value == static_cast<const Leaf&>(other).value;
        };

        T getValue() const {
            return value;
        };
        T& getValueRef() const {
            return value;
        };

    private:
        T value;
    };
}


#endif //NETWORK_DISCOVERY_NODE_H
