/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/bases/EvidenceAware.h>
#include <algorithm>

namespace EFG::nodes {
    typedef std::list<std::set<Node*>>::iterator ClusterIter;
    bool operator<(const ClusterIter& a, const ClusterIter& b) {
        return (*a->begin() < *b->begin());
    }

    HiddenClusters::HiddenClusters(const std::set<Node*>& toSplit) {
        std::set<Node*> openSet = toSplit;
        std::set<ClusterIter> connectedClusters;
        auto itOpen = openSet.begin();
        clusters.push_back({ *itOpen });
        itOpen = openSet.erase(itOpen);
        while (itOpen != openSet.end()) {
            connectedClusters.clear();
            std::set<Node*> newCluster = { *itOpen };
            for (auto itConn = (*itOpen)->activeConnections.begin(); itConn != (*itOpen)->activeConnections.end(); ++itConn) {
                auto clusterIt = this->find(*itConn->first);
                if (clusterIt == clusters.end()) {
                    newCluster.emplace(itConn->first);
                    openSet.erase(openSet.find(itConn->first));
                }
                else {
                    connectedClusters.emplace(clusterIt);
                }
            }
            for (auto conn = connectedClusters.begin(); conn != connectedClusters.end(); ++conn) {
                add2(newCluster, **conn);
                clusters.erase(*conn);
            }
            clusters.push_back(newCluster);
            itOpen = openSet.erase(itOpen);
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
            add2(this->clusters.back(), c);
        });
    }

    void add(std::set<Node*>& recipient, const std::set<Node*>& toAdd) {
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
}
