/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_NODES_H
#define EFG_NODES_NODES_H

#include <distribution/DistributionBase.h>
#include <list>
#include <map>

namespace EFG::nodes {
    struct Connection;

    struct Node {
        categoric::VariablePtr variable;
        std::list<distribution::DistributionPtr> unaryFactors;
        std::map<Node*, Connection> activeConnections;
        // here message to this is the marginalized factor
        std::map<Node*, Connection> disabledConnections;
    };

    struct Connection {
        distribution::DistributionPtr factor;
        //nullptr when the message is not already available
        std::unique_ptr<distribution::DistributionBase> message2This;
    };

    class NodeSet {
    public:
        virtual ~NodeSet() = default;

        categoric::VariablePtr findVariable(const std::string& name) const;

        std::set<categoric::VariablePtr> getVariables() const;

    protected:
        NodeSet() = default;

        std::map<categoric::VariablePtr, Node> nodes;
    };
}

#endif
