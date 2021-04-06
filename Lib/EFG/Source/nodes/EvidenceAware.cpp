/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/EvidenceAware.h>
#include <nodes/Node.h>
#include <Error.h>
#include <algorithm>

namespace EFG::nodes {
    bool Evidence::operator<(const Evidence& o) const {
        return (this->variable < o.variable);
    }

    std::set<categoric::VariablePtr> EvidenceAware::getHiddenVariables() const {
        std::set<categoric::VariablePtr> hiddens;
        std::for_each(this->hiddenClusters.begin(), this->hiddenClusters.end(), [&hiddens](const std::set<Node>& cluster){
            std::for_each(cluster.begin(), cluster.end(), [&hiddens](const Node& n){
                hiddens.emplace(n.getVariable());
            });
        });
        return hiddens;
    }

    void EvidenceAware::setEvidences(const std::set<Evidence>& evidences) {
        // check values are possible
        std::for_each(evidences.begin(), evidences.end(), [this](const Evidence& e) {
            auto itE = this->nodes.find(e.variable);
            if(itE == this->nodes.end()) {
                throw Error("Non existent variable");
            }
            if(e.value >= itE->first->size()) {
                throw Error("Invalid evidence value");
            }
        });

        // build new clusters
        // todo
        throw 0;
    }
}
