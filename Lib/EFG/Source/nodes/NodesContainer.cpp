/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/NodesContainer.h>
#include <algorithm>

namespace EFG::nodes {
    categoric::VariablePtr NodesContainer::findVariable(const std::string& name) const {
        auto it = this->nodes.find(categoric::makeVariable(2, name));
        if (it == this->nodes.end()) {
            return nullptr;
        }
        return it->first;
    }

    std::set<categoric::VariablePtr> NodesContainer::getVariables() const {
        std::set<categoric::VariablePtr> vars;
        for (auto it = this->nodes.begin(); it != this->nodes.end(); ++it) {
            vars.emplace(it->first);
        }
        return vars;
    }

    std::set<categoric::VariablePtr> NodesContainer::getHiddenVariables() const {
        std::set<categoric::VariablePtr> hiddens;
        std::for_each(this->hiddenClusters.begin(), this->hiddenClusters.end(), [&hiddens](const std::set<Node>& cluster) {
            std::for_each(cluster.begin(), cluster.end(), [&hiddens](const Node& n) {
                hiddens.emplace(n.variable);
                });
            });
        return hiddens;
    }
}
