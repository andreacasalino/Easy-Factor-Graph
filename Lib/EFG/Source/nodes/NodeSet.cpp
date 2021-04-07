/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/NodeSet.h>

namespace EFG::nodes {
    categoric::VariablePtr NodeSet::findVariable(const std::string& name) const {
        auto it = this->nodes.find(categoric::makeVariable(2, name));
        if (it == this->nodes.end()) {
            return nullptr;
        }
        return it->first;
    }

    std::set<categoric::VariablePtr> NodeSet::getVariables() const {
        std::set<categoric::VariablePtr> vars;
        for (auto it = this->nodes.begin(); it != this->nodes.end(); ++it) {
            vars.emplace(it->first);
        }
        return vars;
    }
}
