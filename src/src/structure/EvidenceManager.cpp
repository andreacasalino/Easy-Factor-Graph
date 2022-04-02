/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/EvidenceManager.h>

#include "Utils.h"

#include <algorithm>

namespace EFG::strct {
void EvidenceSetter::setEvidence(const categoric::VariablePtr &variable,
                                 const std::size_t value) {
  if (variable->size() <= value) {
    throw Error{std::to_string(value), " is an invalid evidence for variable ",
                variable->name()};
  }
  auto node_location = locate(variable);
  if (std::nullopt == node_location) {
    throw Error{variable->name(), " is a non existing variable"};
  }
  EvidenceNodeLocation evidence_location;
  visit_location(
      *node_location,
      [&](const HiddenNodeLocation &location) {
        auto *node = location.node;
        for (const auto &[connected_node, connection] :
             node->active_connections) {
          node->disabled_connections[connected_node].factor = connection.factor;
          connected_node->disabled_connections[node].factor = connection.factor;
          connected_node->active_connections.erase(node);
        }
        node->active_connections.clear();
        // update clusters
        auto &state = this->getState_();
        if (1 == location.cluster->nodes.size()) {
          state.clusters.erase(location.cluster);
        } else {
          auto nodes = location.cluster->nodes;
          state.clusters.erase(location.cluster);
          nodes.erase(node);
          auto split_clusters = compute_clusters(nodes);
          for (auto &cluster : split_clusters) {
            state.clusters.emplace_back(std::move(cluster));
          }
        }
        evidence_location = EvidenceNodeLocation{
            state.evidences.emplace(node->variable, value).first, node};
      },
      [&evidence_location](const EvidenceNodeLocation &location) {
        evidence_location = location;
      });
  auto *node = evidence_location.node;
  for (auto &[connected_node, connection] : node->disabled_connections) {
    auto connection_it = connected_node->disabled_connections.find(node);
    connection_it->second.message =
        make_evidence(*connection_it->second.factor, node->variable,
                      evidence_location.evidence->second);
    connected_node->merged_unaries.reset();
  }
  resetBelief();
}

namespace {
void re_connect(Node &node, std::set<Node *> &involved_nodes) {
  for (const auto &[connected_node, connection] : node.disabled_connections) {
    involved_nodes.emplace(connected_node);
    node.active_connections[connected_node].factor = connection.factor;
    connected_node->active_connections[&node].factor = connection.factor;
    connected_node->disabled_connections.erase(&node);
    connected_node->merged_unaries.reset();
  }
  involved_nodes.emplace(&node);
  node.merged_unaries.reset();
  node.disabled_connections.clear();
}

void update_clusters(HiddenClusters &clusters,
                     std::set<Node *> &involved_nodes) {
  for (auto *node : involved_nodes) {
    auto clusters_it =
        std::find_if(clusters.begin(), clusters.end(),
                     [&node](const HiddenCluster &element) {
                       return element.nodes.find(node) != element.nodes.end();
                     });
    if (clusters_it != clusters.end()) {
      clusters.erase(clusters_it);
    }
  }
  auto new_clusters = compute_clusters(involved_nodes);
  for (auto &cluster : new_clusters) {
    clusters.emplace_back(std::move(cluster));
  }
}
} // namespace

void EvidenceRemover::removeEvidence(const categoric::VariablePtr &variable) {
  auto &state = getState_();
  auto evidence_it = state.evidences.find(variable);
  if (evidence_it == state.evidences.end()) {
    throw Error{variable->name(), " is not an evidence"};
  }
  resetBelief();
  state.evidences.erase(evidence_it);
  std::set<Node *> involved_nodes;
  re_connect(state.nodes[variable], involved_nodes);
  update_clusters(state.clusters, involved_nodes);
}

void EvidenceRemover::removeEvidences() {
  auto &state = getState_();
  if (state.evidences.empty()) {
    return;
  }
  resetBelief();
  std::set<Node *> involved_nodes;
  for (const auto &[var, val] : state.evidences) {
    re_connect(state.nodes[var], involved_nodes);
  }
  state.evidences.clear();
  update_clusters(state.clusters, involved_nodes);
}
} // namespace EFG::strct
