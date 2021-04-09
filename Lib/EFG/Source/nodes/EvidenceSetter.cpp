/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/EvidenceSetter.h>
#include <distribution/FactorConst.h>
#include <Error.h>
#include <algorithm>

namespace EFG::nodes {
    void disconnect(Node& node, std::size_t value) {
        for (auto it = node.activeConnections.begin(); it != node.activeConnections.end(); ++it) {
            auto toDisable = it->first->activeConnections.find(&node);
            distribution::DistributionPtr factor = toDisable->second.factor;

            node.disabledConnections.emplace(it->first, factor);

            it->first->disabledConnections.emplace(&node, factor , std::make_unique<distribution::FactorConst>(factor, distribution::Combination({ value }), categoric::Group(node.variable)));
            it->first->activeConnections.erase(toDisable);
        }
        node.activeConnections.clear();
    }

    void EvidencesSetter::addEvidence(const std::string& name, std::size_t value) {
        auto itN = this->nodes.find(categoric::makeVariable(2 , name));
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
        disconnect(itN->second, value);
        this->evidences.emplace(itN->second, value);
        // split cluster cause one node is gone
        HiddenClusters splitted(*itCluster);
        this->hidden.clusters.erase(itCluster);
        this->hidden.add(splitted.clusters);
    }

    void reconnnect(Node& node) {
        for (auto it = node.disabledConnections.begin(); it != node.disabledConnections.end(); ++it) {
            auto toEnable = it->first->disabledConnections.find(&node);
            distribution::DistributionPtr factor = toEnable->second.factor;

            node.activeConnections.emplace(toEnable->first, factor);
            it->first->activeConnections.emplace(&node, factor);
        }
        node.disabledConnections.clear();
    }

    void EvidencesSetter::setEvidences(const std::map<std::string, const std::size_t>& evidences) {
        std::map<Node*, const std::size_t> obs;
        // check values
        for (auto it = evidences.begin(); it != evidences.end(); ++it) {
            auto itN = this->nodes.find(categoric::makeVariable(2 , it->first));
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
    }

    void EvidencesSetter::setEvidences(const std::vector<std::size_t>& observations) {
        if (this->evidences.size() != observations.size()) {
            throw Error("invalid number of observations");
        }
        // check size
        auto itOb = observations.begin();
        for (auto it = this->evidences.begin(); it != this->evidences.end(); ++it) {
            if (*itOb >= it->first->size()) {
                throw Error("invalid evidence value for variable " + it->first->name());
            }
            ++itOb;
        }
        itOb = observations.begin();
        for (auto it = this->evidences.begin(); it != this->evidences.end(); ++it) {
            auto itN = this->nodes.find(it->first);
            reconnnect(itN->second);
            disconnect(itN->second, *itOb);
            ++itOb;
        }
    }
}
