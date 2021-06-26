/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <structure/components/NodesAware.h>
#include <algorithm>

namespace EFG::strct {
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

    std::set<distribution::DistributionCnstPtr> NodesAware::getAllFactors() const {
        std::set<distribution::DistributionCnstPtr> factors;
        for (auto it = this->nodes.begin(); it != this->nodes.end(); ++it) {
            std::for_each(it->second.unaryFactors.begin(), it->second.unaryFactors.end(), [&factors](const distribution::DistributionPtr& d) {
                factors.emplace(d);
            });
            for (auto itA = it->second.activeConnections.begin(); itA != it->second.activeConnections.end(); ++itA) {
                factors.emplace(itA->second.factor);
            }
            for (auto itD = it->second.disabledConnections.begin(); itD != it->second.disabledConnections.end(); ++itD) {
                factors.emplace(itD->second.factor);
            }
        }
        return factors;
    }
}
