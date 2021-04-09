/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_NODE_H
#define EFG_NODES_NDOE_H

#include <distribution/DistributionBase.h>
#include <list>

namespace EFG::nodes {
    struct Connection {
        Connection(distribution::DistributionPtr factor, std::unique_ptr<distribution::DistributionBase> message = nullptr)
            : factor(factor)
            , message2This(std::move(message)) {
        };

        distribution::DistributionPtr factor;
        //nullptr when the message is not already available
        std::unique_ptr<distribution::DistributionBase> message2This;
    };

    struct Node {
        Node(categoric::VariablePtr var) { variable = var; };

        categoric::VariablePtr variable;
        std::list<distribution::DistributionPtr> unaryFactors;
        std::map<Node*, Connection> activeConnections;
        // here message to this is the marginalized factor
        std::map<Node*, Connection> disabledConnections;
    };
}

#endif
