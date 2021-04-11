/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_NODES_AWARE_H
#define EFG_NODES_NODES_AWARE_H

#include <nodes/Node.h>
#include <nodes/bases/Base.h>

namespace EFG::nodes {
    class NodesAware : virtual public Base {
    public:
        std::set<categoric::VariablePtr> getVariables() const;

        categoric::VariablePtr findVariable(const std::string& name) const;

    protected:
        inline std::map<categoric::VariablePtr, Node>::const_iterator findNodeConst(const std::string& name) const { return this->nodes.find(categoric::makeVariable(2, name)); };
        inline std::map<categoric::VariablePtr, Node>::iterator findNode(const std::string& name) { return this->nodes.find(categoric::makeVariable(2, name)); };

        std::map<categoric::VariablePtr, Node> nodes;
    };
}

#endif
