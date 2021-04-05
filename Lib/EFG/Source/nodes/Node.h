/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NDOES_NODE_H
#define EFG_NDOES_NODE_H

#include <nodes/NodesBase.h>
#include <list>

namespace EFG::nodes {
    class Node {
    public:
        class Connection;

        Node(categoric::VariablePtr variable);

        inline const categoric::Variable& getVariable() const { return *this->variable; };
        inline const std::list<distribution::DistributionPtr>& getUnaryFactors() { return this->unaryFactors; };
        inline const std::set<Connection>& getConnections() const { return this->connections; };

        inline void addUnary(distribution::DistributionPtr unaryFactor) { this->unaryFactors.push_back(unaryFactor); };

    private:
        categoric::VariablePtr variable;
        std::list<distribution::DistributionPtr> unaryFactors;
        std::set<Connection> connections;
    };


    bool operator<(const Node::Connection& a, const Node::Connection& b);
}

#endif
