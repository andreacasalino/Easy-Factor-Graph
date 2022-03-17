/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include "GraphStateUtils.h"
#include <EasyFactorGraph/structure/EvidenceAware.h>

namespace EFG::strct {
std::unordered_set<categoric::VariablePtr>
EvidenceAware::getHiddenVariables() const {
  std::unordered_set<categoric::VariablePtr> result;
  for (const auto &cluster : state->hidden_clusters) {
    for (const auto *node : cluster) {
      result.emplace(node->variable);
    }
  }
  return result;
}

std::unordered_set<categoric::VariablePtr>
EvidenceAware::getObservedVariables() const {
  std::unordered_set<categoric::VariablePtr> result;
  for (const auto &[var, val] : state->evidences) {
    result.emplace(var);
  }
  return result;
}

namespace {
std::vector<HiddenCluster> remove_node(Node &node,
                                       const HiddenCluster &cluster);
}

void EvidenceAware::setEvidence(Node &node, const std::size_t value,
                                const bool observation_should_prexist) {
  if (node.variable->size() >= value) {
    throw Error{std::to_string(value), " is an invalid evidence for variable ",
                node.variable->name()};
  }
  auto evidence_it = state->evidences.find(node.variable);
  if (evidence_it == state->evidences.end()) {
    if (observation_should_prexist) {
      throw Error{node.variable->name(), " can't be set as evidence"};
    }
    auto info = find_node(*state, node);
    auto &info_as_hidden = std::get<std::vector<HiddenCluster>::iterator>(info);
    if (1 == info_as_hidden->size()) {
      state->hidden_clusters.erase(info_as_hidden);
    } else {
      auto updated_clusters = remove_node(node, *info_as_hidden);
      auto updated_clusters_it = updated_clusters.begin();
      *info_as_hidden = std::move(*updated_clusters_it);
      ++updated_clusters_it;
      state->hidden_clusters.insert(state->hidden_clusters.end(),
                                    updated_clusters_it,
                                    updated_clusters.end());
    }
    state->evidences.emplace(node.variable->name(), value).first;
    for (const auto &[connected_node, connection] : node.activeConnections) {
      disable_connection(node, *connected_node);
    }
  }
  for (auto &[connected_node, connection] : node.disabledConnections) {
    connected_node->disabledConnections[&node].factor =
        make_evidence_message(connection.factor, node.variable, value);
  }
}

namespace {
void add_node(Node &node, std::vector<HiddenCluster> &clusters);
}

void EvidenceAware::resetEvidence(Node &node) {
  auto evidence_it = state->evidences.find(node.variable);
  if (evidence_it == state->evidences.end()) {
    throw Error{node.variable->name(), " is not an evidence"};
  }
  state->evidences.erase(evidence_it);
  for (const auto &[connected_node, connection] : node.disabledConnections) {
    enable_connection(node, *connected_node);
  }
  add_node(node, state->hidden_clusters);
}

void EvidenceAware::resetEvidences() {
  std::vector<Node *> nodes;
  nodes.reserve(state->evidences.size());
  for (const auto &[var, val] : state->evidences) {
    nodes.push_back(&state->nodes[var]);
  }
  for (auto *node : nodes) {
    resetEvidence(*node);
  }
}
} // namespace EFG::strct
