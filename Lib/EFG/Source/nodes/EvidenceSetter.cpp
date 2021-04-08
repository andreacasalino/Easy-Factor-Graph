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
    typedef std::list<std::set<Node*>>::iterator ClusterIterator;

    ClusterIterator findCluster(Node& node, std::list<std::set<Node*>>& clusters) {
        for (auto itCl = clusters.begin(); itCl != clusters.end(); ++itCl) {
            if (itCl->find(&node) != itCl->end()) {
                return itCl;
            }
        }
        return clusters.end();
    }

    std::list<std::set<Node*>> splitClusters(std::set<Node*>& toSplit) {
        std::list<ClusterIterator> connectedClusters;
        auto itOpen = toSplit.begin();
        std::list<std::set<Node*>> clusters;
        clusters.push_back({ *itOpen });
        itOpen = toSplit.erase(itOpen);
        while (itOpen != toSplit.end()) {
            connectedClusters.clear();
            std::set<Node*> newCluster = { *itOpen };
            for (auto itConn = (*itOpen)->activeConnections.begin(); itConn != (*itOpen)->activeConnections.end(); ++itConn) {
                auto clusterIt = findCluster(*itConn->first, clusters);
                if (clusterIt == clusters.end()) {
                    newCluster.emplace(itConn->first);
                    toSplit.erase(toSplit.find(itConn->first));
                }
                else {
                    connectedClusters.push_back(clusterIt);
                }

                for (auto conn = connectedClusters.begin(); conn != connectedClusters.end(); ++conn) {
                    transfer(newCluster, *conn);
                    clusters.erase(*conn);
                }
                clusters.push_back(newCluster);
            }
            itOpen = toSplit.erase(itOpen);
        }
    };

    void disconnect(Node& node, std::size_t value) {
        for (auto it = node.activeConnections.begin(); it != node.activeConnections.end(); ++it) {
            auto toDisable = it->first->activeConnections.find(&node);
            distribution::DistributionPtr factor = toDisable->second.factor;

            node.disabledConnections.emplace(it->first, Connection{ factor , nullptr });

            std::unique_ptr<distribution::FactorConst> marginalized = std::make_unique<distribution::FactorConst>(toDisable->second.factor, distribution::Combination({ value }), categoric::Group(node.variable));
            it->first->disabledConnections.emplace(&node, Connection{ factor , std::move(marginalized) });
            it->first->activeConnections.erase(toDisable);
        }
        node.activeConnections.clear();
    }

    void EvidencesSetter::addEvidence(categoric::VariablePtr variable, std::size_t value) {
        auto itN = this->nodes.find(variable);
        if (itN == this->nodes.end()) {
            throw Error("Inexistent variable");
        }
        auto itCluster = findCluster(itN->second, this->hiddenClusters);
        if (itCluster == this->hiddenClusters.end()) {
            throw Error("That variable is already an evidence");
        }
        if (value >= itN->second.variable->size()) {
            throw Error("invalid evidence value");
        }
        disconnect(itN->second, value);
        this->evidences.emplace(itN->second, value);
        // split cluster cause one node is gone
        auto splitted = splitClusters(*itCluster);
        this->hiddenClusters.erase(itCluster);
        std::for_each(splitted.begin(), splitted.end(), [this](const std::set<Node*>& s) {
            this->hiddenClusters.push_back(s);
        });
    }

    void reconnnect(Node& node) {
        for (auto it = node.disabledConnections.begin(); it != node.disabledConnections.end(); ++it) {
            auto toEnable = it->first->disabledConnections.find(&node);
            distribution::DistributionPtr factor = toEnable->second.factor;

            node.activeConnections.emplace(toEnable->first, Connection{ factor , nullptr });
            it->first->activeConnections.emplace(&node, Connection{ factor , nullptr });
        }
        node.disabledConnections.clear();
    }

    void transfer(std::set<Node*>& recipient, const ClusterIterator& it) {
        std::for_each(it->begin(), it->end(), [&recipient](Node* n) {
            recipient.emplace(n);
        });
    }

    void EvidencesSetter::setEvidences(const std::map<categoric::VariablePtr, const std::size_t>& evidences) {
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
        std::set<Node*> hiddenVars;
        for (auto it = this->nodes.begin(); it != this->nodes.end(); ++it) {
            if (this->evidences.find(it->first) == this->evidences.end()) {
                // hidden var
                hiddenVars.emplace(&it->second);
            }
        }
        this->hiddenClusters = splitClusters(hiddenVars);
    }

    void EvidencesSetter::setEvidences(const std::list<std::size_t>& observations) {
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
