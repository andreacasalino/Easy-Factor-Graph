/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <structure/components/EvidenceAware.h>
#include <algorithm>

namespace EFG::strct {
    typedef std::list<std::set<Node*>>::iterator ClusterIter;
    inline bool operator<(const ClusterIter& a, const ClusterIter& b) {
        return (*a->begin() < *b->begin());
    };

    HiddenClusters::HiddenClusters(const std::set<Node*>& toSplit) {
        auto itOpen = toSplit.begin();
        this->clusters.push_back({ *itOpen });
        ++itOpen;
        for(itOpen; itOpen != toSplit.end(); ++itOpen) {
            std::set<ClusterIter> connectedClusters;
            for (auto itConn = (*itOpen)->activeConnections.begin(); itConn != (*itOpen)->activeConnections.end(); ++itConn) {
                auto clusterIt = this->find(*itConn->first);
                if(clusterIt != this->clusters.end()) {
                    connectedClusters.emplace(clusterIt);
                }
            }
            if(connectedClusters.empty()) {
                // this the root of a new cluster
                this->clusters.push_back({ *itOpen });
            }
            else {
                // merge all connected clusters together
                auto conn = connectedClusters.begin();
                auto connFirst = conn;
                (*connFirst)->emplace(*itOpen);
                ++conn;
                for (conn; conn != connectedClusters.end(); ++conn) {
                    copyCluster(**connFirst, **conn);
                    this->clusters.erase(*conn);
                }
            }
        }
    }

    std::list<std::set<Node*>>::iterator HiddenClusters::find(Node& node) {
        for (auto itCl = clusters.begin(); itCl != clusters.end(); ++itCl) {
            if (itCl->find(&node) != itCl->end()) {
                return itCl;
            }
        }
        return clusters.end();
    }

    void HiddenClusters::add(const std::list<std::set<Node*>>& toAdd) {
        std::for_each(toAdd.begin(), toAdd.end(), [this](const std::set<Node*>& c) {
            this->clusters.emplace_back();
            copyCluster(this->clusters.back(), c);
        });
    }

    void copyCluster(std::set<Node*>& recipient, const std::set<Node*>& toAdd) {
        std::for_each(toAdd.begin(), toAdd.end(), [&recipient](Node* n) {
            recipient.emplace(n);
        });
    }

    std::set<categoric::VariablePtr> EvidenceAware::getHiddenVariables() const {
        std::set<categoric::VariablePtr> hiddens;
        std::for_each(this->hidden.clusters.begin(), this->hidden.clusters.end(), [&hiddens](const std::set<Node*>& cluster) {
            std::for_each(cluster.begin(), cluster.end(), [&hiddens](const Node* n) {
                hiddens.emplace(n->variable);
            });
        });
        return hiddens;
    }

    std::set<categoric::VariablePtr> EvidenceAware::getObservedVariables() const {
        std::set<categoric::VariablePtr> observed;
        for (auto it = this->evidences.begin(); it != this->evidences.end(); ++it) {
            observed.emplace(it->first);
        }
        return observed;
    }
}
