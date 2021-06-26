/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_STRUCTURE_NODE_H
#define EFG_STRUCTURE_NODE_H

#include <distribution/Distribution.h>
#include <list>

namespace EFG::strct {
    struct Connection {
        Connection(distribution::DistributionPtr factor, std::unique_ptr<distribution::Distribution> message = nullptr);
        Connection(Connection&& o);

        distribution::DistributionPtr factor;
        // nullptr when the message is not already available
        std::unique_ptr<distribution::Distribution> message2This;
    };

    struct Node {
        explicit Node(categoric::VariablePtr var);

        categoric::VariablePtr variable;
        std::list<distribution::DistributionPtr> unaryFactors;
        std::map<Node*, Connection> activeConnections;
        // here message to this is the marginalized factor
        std::map<Node*, Connection> disabledConnections;
    };
}

#endif
