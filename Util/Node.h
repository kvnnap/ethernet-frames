//
// Created by kevin on 9/15/16.
//

#ifndef NETWORK_DISCOVERY_NODE_H
#define NETWORK_DISCOVERY_NODE_H

#include <vector>
#include <set>
#include <memory>
#include <sstream>
#include "Ethernet/MacAddress.h"

namespace Util {
    enum NodeType {
        NODE,
        LEAF
    };

    class AbstractNode;
    class Node;
    class Leaf;
    using NodePt = std::unique_ptr<AbstractNode>;

    class AbstractNode {
    public:

        AbstractNode();
        virtual ~AbstractNode();

        virtual NodeType getType() const = 0;

        // This is architectural equality, need to define another type
        virtual bool operator== (const AbstractNode& other) const = 0;
        virtual bool weakEquality(const AbstractNode& other) const = 0;
        bool unrootedWeakEquality(NodePt& other) const;

        virtual std::set<Network::MacAddress> getValues() const = 0;
        virtual std::vector<Node*> getNodes() = 0;
        virtual Leaf* findNode(const Network::MacAddress& value) = 0;
        bool deleteValue(const Network::MacAddress& value);

        // To Dot string
        std::string toDot() const;
        virtual std::string toDotEdges(size_t& labelNum) const = 0;

        // To Dot File
        void toDotFile(const std::string& fileName) const;

        // Access parent
        void setParent(Node * p_parent);
        Node * getParent();
    private:
        // No unique_ptr needed, this reference will always be true
        Node * parent;
    };

}
namespace Util {
    class Node
        : public AbstractNode
    {
    public:

        NodeType getType() const override;

        bool operator== (const AbstractNode& other) const override;
        bool weakEquality(const AbstractNode& other) const override;

        std::set<Network::MacAddress> getValues() const override;
        std::vector<Node*> getNodes() override;
        Leaf* findNode(const Network::MacAddress& value) override;
        NodePt deleteChild(const AbstractNode* node);
        void makeRoot(NodePt& treeOwner);

        std::string toDotEdges(size_t& labelNum) const override;

        const std::vector<NodePt>& getChildren() const;
        size_t getChildrenSize() const;
        void addChild(NodePt child);

    private:
        std::vector<NodePt> children;
    };
}

namespace Util {
    class Leaf
        : public AbstractNode
    {
    public:

        Leaf(const Network::MacAddress& value);

        NodeType getType() const override;

        bool operator== (const AbstractNode& other) const override;
        bool weakEquality(const AbstractNode& other) const override;

        std::set<Network::MacAddress> getValues() const override;
        std::vector<Node*> getNodes() override;
        Leaf* findNode(const Network::MacAddress& value) override;

        std::string toDotEdges(size_t& labelNum) const override;

        Network::MacAddress getValue() const;
        const Network::MacAddress& getValueRef() const;

    private:
        Network::MacAddress value;
    };
}


#endif //NETWORK_DISCOVERY_NODE_H
