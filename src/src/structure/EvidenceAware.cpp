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
                                       const HiddenCluster &cluster) {
  HiddenCluster cluster_modified = cluster;
  cluster_modified.erase(&node);
  return split_cluster(cluster_modified);
};
} // namespace

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
      *info_as_hidden = std::move(updated_clusters.front());
      for (std::size_t k = 1; k < updated_clusters.size(); ++k) {
        state->hidden_clusters.emplace_back(std::move(updated_clusters[k]));
      }
    }
    state->evidences.emplace(node.variable->name(), value);
    for (const auto &[connected_node, connection] : node.activeConnections) {
      disable_connection(node, *connected_node);
    }
  }
  resetBelief();
  for (auto &[connected_node, connection] : node.disabledConnections) {
    connected_node->disabledConnections[&node].factor =
        make_evidence_message(connection.factor, node.variable, value);
  }
}

namespace {
HiddenCluster gather_hidden(const std::vector<HiddenCluster> &clusters) {
  HiddenCluster result;
  for (const auto &cluster : clusters) {
    result.insert(cluster.begin(), cluster.end());
  }
  return result;
}

void enable_connections(Node &subject) {
  for (auto &[connected_node, connection] : subject.disabledConnections) {
    enable_connection(subject, *connected_node);
  }
}
} // namespace

void EvidenceAware::removeEvidence(Node &node) {
  auto evidence_it = state->evidences.find(node.variable);
  if (evidence_it == state->evidences.end()) {
    throw Error{node.variable->name(), " is not an evidence"};
  }
  resetBelief();
  state->evidences.erase(evidence_it);
  enable_connections(node);
  auto hidden_nodes = gather_hidden(state->hidden_clusters);
  hidden_nodes.emplace(&node);
  state->hidden_clusters = split_cluster(hidden_nodes);
}

void EvidenceAware::removeEvidences() {
  if (state->evidences.empty()) {
    return;
  }
  resetBelief();
  std::vector<Node *> nodes;
  nodes.reserve(state->evidences.size());
  for (const auto &[var, val] : state->evidences) {
    nodes.push_back(&state->nodes[var]);
    enable_connections(*nodes.back());
  }
  auto hidden_nodes = gather_hidden(state->hidden_clusters);
  hidden_nodes.insert(nodes.begin(), nodes.end());
  state->hidden_clusters = split_cluster(hidden_nodes);
}
} // namespace EFG::strct