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

void EvidenceRemover::removeEvidence_(const categoric::VariablePtr &variable) {
  auto &state = getState_();
  auto evidence_it = state.evidences.find(variable);
  if (evidence_it == state.evidences.end()) {
    throw Error{variable->name(), " is not an evidence"};
  }
  resetBelief();
  state.evidences.erase(evidence_it);
  auto &node = state.nodes[variable];
  for (const auto &[connected_node, connection] : node.disabled_connections) {
    node.active_connections[connected_node].factor = connection.factor;
    connected_node->active_connections[&node].factor = connection.factor;
    connected_node->disabled_connections.erase(&node);
    connected_node->merged_unaries.reset();
  }
  node.merged_unaries.reset();
  node.disabled_connections.clear();
}

void EvidenceRemover::removeEvidence(const categoric::VariablePtr &variable) {
  removeEvidence_(variable);
  resetState();
}

void EvidenceRemover::removeEvidences(
    const categoric::VariablesSet &variables) {
  if (variables.empty()) {
    return;
  }
  for (const auto &variable : variables) {
    removeEvidence_(variable);
  }
  resetState();
}

void EvidenceRemover::removeAllEvidences() {
  const auto &state = getState();
  while (!state.evidences.empty()) {
    removeEvidence_(state.evidences.begin()->first);
  }
  resetState();
}

void EvidenceRemover::resetState() {
  auto &state = getState_();
  std::set<Node *> nodes;
  for (auto &[var, node] : state.nodes) {
    if (state.evidences.find(var) == state.evidences.end()) {
      nodes.emplace(&node);
    }
  }
  state.clusters = compute_clusters(nodes);
}
} // namespace EFG::strct
