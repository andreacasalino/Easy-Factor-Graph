/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/EvidenceAware.h>
#include <distribution/FactorConst.h>
#include <Error.h>
#include <algorithm>

namespace EFG::nodes {
    std::set<categoric::VariablePtr> EvidenceAware::getHiddenVariables() const {
        std::set<categoric::VariablePtr> hiddens;
        std::for_each(this->hiddenClusters.begin(), this->hiddenClusters.end(), [&hiddens](const std::set<Node>& cluster){
            std::for_each(cluster.begin(), cluster.end(), [&hiddens](const Node& n){
                hiddens.emplace(n.variable);
            });
        });
        return hiddens;
    }

    void disconnect(Node& node, std::size_t value) {
        for (auto it = node.activeConnections.begin(); it != node.activeConnections.end(); ++it) {
            auto toDisable = it->first->activeConnections.find(&node);
            std::unique_ptr<distribution::FactorConst> marginalized = std::make_unique<distribution::FactorConst>(toDisable->second.factor, distribution::Combination({value}), categoric::Group(node.variable));
            it->first->disabledConnections.emplace(&node, Connection{ toDisable->second.factor , std::move(marginalized)});
            node.disabledConnections.emplace(it->first, Connection{ toDisable->second.factor , nullptr });
            it->first->activeConnections.erase(toDisable);
        }
        node.activeConnections.clear();
    }

    void reconnnect(Node& node) {
        for (auto it = node.disabledConnections.begin(); it != node.disabledConnections.end(); ++it) {
            auto toRemove = it->first->disabledConnections.find(&node);
            node.activeConnections.emplace(toRemove->first, Connection{ toRemove->second.factor , nullptr });
            it->first->activeConnections.emplace(&node, Connection{ toRemove->second.factor , nullptr });
        }
        node.disabledConnections.clear();
    }

    typedef std::list<std::set<Node*>>::iterator ClusterIterator;
    ClusterIterator find(std::list<std::set<Node*>>& clusters, Node* n) {
        for (auto it = clusters.begin(); it != clusters.end(); ++it) {
            if (it->find(n) != it->end()) {
                return it;
            }
        }
        return clusters.end();
    };
    void transfer(std::set<Node*>& recipient, const ClusterIterator& it) {
        std::for_each(it->begin(), it->end(), [&recipient](Node* n) {
            recipient.emplace(n);
        });
    }
    void EvidenceAware::recomputeHiddenClusters() {
        this->hiddenClusters.clear();
        std::set<Node*> openSet;
        for (auto it = this->nodes.begin(); it != this->nodes.end(); ++it) {
            if (this->evidences.find(it->first) == this->evidences.end()) {
                // hidden var
                openSet.emplace(&it->second);
            }
        }
        if (openSet.empty()) {
            return;
        }
        
        std::list<ClusterIterator> connectedClusters;
        auto itOpen = openSet.begin();
        this->hiddenClusters.push_back({*itOpen});
        itOpen = openSet.erase(itOpen);
        while (itOpen != openSet.end()) {
            connectedClusters.clear();
            std::set<Node*> newCluster = {*itOpen};
            for (auto itConn = (*itOpen)->activeConnections.begin(); itConn != (*itOpen)->activeConnections.end(); ++itConn) {
                auto clusterIt = find(this->hiddenClusters, itConn->first);
                if (clusterIt == this->hiddenClusters.end()) {
                    newCluster.emplace(itConn->first);
                    openSet.erase(openSet.find(itConn->first));
                }
                else {
                    connectedClusters.push_back(clusterIt);
                }

                for (auto conn = connectedClusters.begin(); conn != connectedClusters.end(); ++conn) {
                    transfer(newCluster, *conn);
                    this->hiddenClusters.erase(*conn);
                }
                this->hiddenClusters.push_back(newCluster);
            }
            itOpen = openSet.erase(itOpen);
        }
    }

    void EvidenceAware::addEvidence(categoric::VariablePtr variable, std::size_t value) {
        if (this->evidences.find(variable) != this->evidences.end()) {
            throw Error("evidence already added");
        }
        auto itN = this->nodes.find(variable);
        if (itN == this->nodes.end()) {
            throw Error("inexistent variable");
        }
        if (value >= itN->second.variable->size()) {
            throw Error("invalid evidence value");
        }
        disconnect(itN->second, value);
        this->evidences.emplace(itN->second, value);
        this->recomputeHiddenClusters();
    }

    void EvidenceAware::setEvidences(const std::map<categoric::VariablePtr, const std::size_t>& evidences) {
        std::map<Node*, const std::size_t> obs;
        // check values
        for (auto it = evidences.begin(); it != evidences.end(); ++it) {
            auto itN = this->nodes.find(it->first);
            if (itN == this->nodes.end()) {
                throw Error("inexistent variable");
            }
            if (it->second >= itN->first->size()) {
                throw Error("invalid evidence value for variable " + it->first->name());
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
        this->recomputeHiddenClusters();
    }
}
