//
// Created by kevin on 9/15/16.
//

#ifndef NETWORK_DISCOVERY_NODE_H
#define NETWORK_DISCOVERY_NODE_H

#include <vector>
#include <memory>
#include <sstream>
#include "Ethernet/MacAddress.h"

namespace Util {
    enum NodeType {
        NODE,
        LEAF
    };

    class AbstractNode {
    public:

        virtual ~AbstractNode();

        virtual NodeType getType() const = 0;
        // This is architectural equality, need to define another type
        virtual bool operator== (const AbstractNode& other) const = 0;
        //virtual bool deleteValue(const Network::MacAddress& value) = 0;

        // To Dot string
        std::string toDot() const;
        virtual std::string toDotEdges(size_t& labelNum) const = 0;

        // To Dot File
        void toDotFile(const std::string& fileName) const;
    };

    using NodePt = std::unique_ptr<AbstractNode>;
}
namespace Util {
    class Node
        : public AbstractNode
    {
    public:

        NodeType getType() const override;

        virtual bool operator== (const AbstractNode& other) const override;
        //virtual bool deleteValue(const Network::MacAddress& value) override;

        virtual std::string toDotEdges(size_t& labelNum) const override;

        const std::vector<NodePt>& getChildren() const;
        size_t getChildrenSize() const;
        void addChild(NodePt child);

    private:
        std::vector<NodePt> children;
    };
}

namespace Util {
    //template <class T>
    class Leaf
        : public AbstractNode
    {
    public:

        Leaf(const Network::MacAddress& value);

        NodeType getType() const override;

        virtual bool operator== (const AbstractNode& other) const override;
        //virtual bool deleteValue(const Network::MacAddress& value) override;

        virtual std::string toDotEdges(size_t& labelNum) const override;

        Network::MacAddress getValue() const;
        const Network::MacAddress& getValueRef() const;

    private:

        Network::MacAddress value;
    };
}


#endif //NETWORK_DISCOVERY_NODE_H
