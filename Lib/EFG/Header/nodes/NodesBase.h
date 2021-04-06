/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_BASE_H
#define EFG_NODES_BASE_H

#include <distribution/DistributionBase.h>
#include <map>

namespace EFG::nodes {
    class Node;

    class NodesBase {
    public:
        virtual ~NodesBase() = default;

        categoric::VariablePtr findVariable(const std::string& name) const;

    protected:
        NodesBase() = default;

        // virtual const std::set<distribution::DistributionPtr>& getBinaryFactorS() = 0; 
        // virtual const std::set<distribution::DistributionPtr>& getUnaryFactorS() = 0; 
        // virtual const std::set<Node>& getNodes() = 0;

        std::map<categoric::VariablePtr, Node> nodes;
    };
}

#endif
