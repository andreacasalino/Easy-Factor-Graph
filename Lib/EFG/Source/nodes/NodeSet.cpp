/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/Nodes.h>

namespace EFG::nodes {
    categoric::VariablePtr NodeSet::findVariable(const std::string& name) const {
        auto it = this->nodes.find(categoric::makeVariable(2, name));
        if (it == this->nodes.end()) {
            return nullptr;
        }
        return it->first;
    }
}
