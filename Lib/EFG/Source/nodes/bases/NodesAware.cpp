/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/bases/NodesAware.h>
#include <algorithm>

namespace EFG::nodes {
    std::set<categoric::VariablePtr> NodesAware::getVariables() const {
        std::set<categoric::VariablePtr> vars;
        for (auto it = this->nodes.begin(); it != this->nodes.end(); ++it) {
            vars.emplace(it->first);
        }
        return vars;
    }

    categoric::VariablePtr NodesAware::findVariable(const std::string& name) const {
        auto it = this->nodes.find(categoric::makeVariable(2, name));
        if (it == this->nodes.end()) {
            return nullptr;
        }
        return it->first;
    }
}
