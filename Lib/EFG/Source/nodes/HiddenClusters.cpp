/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/NodesContainer.h>
#include <algorithm>

namespace EFG::nodes {
    NodesContainer::HiddenClusters::HiddenClusters(const std::set<Node*>& toSplit) {
        std::set<Node*> openSet = toSplit;
        std::list<std::list<std::set<Node*>>::iterator> connectedClusters;
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
                    connectedClusters.push_back(clusterIt);
                }

                for (auto conn = connectedClusters.begin(); conn != connectedClusters.end(); ++conn) {
                    NodesContainer::add(newCluster, **conn);
                    clusters.erase(*conn);
                }
                clusters.push_back(newCluster);
            }
            itOpen = openSet.erase(itOpen);
        }
    }

    std::list<std::set<Node*>>::iterator NodesContainer::HiddenClusters::find(Node& node) {
        for (auto itCl = clusters.begin(); itCl != clusters.end(); ++itCl) {
            if (itCl->find(&node) != itCl->end()) {
                return itCl;
            }
        }
        return clusters.end();
    }

    void NodesContainer::HiddenClusters::add(const std::list<std::set<Node*>>& toAdd) {
        std::for_each(toAdd.begin(), toAdd.end(), [this](const std::set<Node*>& c) {
            this->clusters.emplace_back();
            NodesContainer::add(this->clusters.back(), c);
        });
    }

    void NodesContainer::add(std::set<Node*>& recipient, const std::set<Node*>& toAdd) {
        std::for_each(toAdd.begin(), toAdd.end(), [&recipient](Node* n) {
            recipient.emplace(n);
        });
    }
}
