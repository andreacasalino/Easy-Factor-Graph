/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <structure/EvidenceChanger.h>
#include <Error.h>
#include "Commons.h"

namespace EFG::strct {
    void EvidencesChanger::addEvidence(const std::string& name, std::size_t value) {
        auto itN = this->nodes.find(categoric::makeVariable(2, name));
        if (itN == this->nodes.end()) {
            throw Error("Inexistent variable");
        }
        auto itCluster = this->hidden.find(itN->second);
        if (itCluster == this->hidden.clusters.end()) {
            throw Error("That variable is already an evidence");
        }
        if (value >= itN->second.variable->size()) {
            throw Error("invalid evidence value");
        }
        this->lastPropagation.reset();
        disconnect(itN->second, value);
        this->evidences.emplace(itN->first, value);
        itCluster->erase(&itN->second);
        if (itCluster->empty()) {
            this->hidden.clusters.erase(itCluster);
            return;
        }
        // split cluster cause one node is gone
        HiddenClusters splitted(*itCluster);
        this->hidden.clusters.erase(itCluster);
        this->hidden.add(splitted.clusters);
    }

    void EvidencesChanger::resetEvidences(const std::map<std::string, std::size_t>& evidences) {
        std::map<Node*, const std::size_t> obs;
        // check values
        for (auto it = evidences.begin(); it != evidences.end(); ++it) {
            auto itN = this->nodes.find(categoric::makeVariable(2, it->first));
            if (itN == this->nodes.end()) {
                throw Error("inexistent variable");
            }
            if (it->second >= itN->first->size()) {
                throw Error("invalid evidence value for variable " + it->first);
            }
            obs.emplace(&itN->second, it->second);
        }
        // update connections
        for (auto it = this->evidences.begin(); it != this->evidences.end(); ++it) {
            reconnnect(this->nodes.find(it->first)->second);
        }
        this->evidences.clear();
        for (auto it = obs.begin(); it != obs.end(); ++it) {
            disconnect(*it->first, it->second);
            this->evidences.emplace(it->first->variable, it->second);
        }
        std::set<Node*> hiddenVars;
        for (auto it = this->nodes.begin(); it != this->nodes.end(); ++it) {
            if (this->evidences.find(it->first) == this->evidences.end()) {
                // hidden var
                hiddenVars.emplace(&it->second);
            }
        }
        this->hidden = HiddenClusters(hiddenVars);
        this->lastPropagation.reset();
    }
}
