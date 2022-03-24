/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include "GraphStateUtils.h"
#include <EasyFactorGraph/structure/EvidenceAware.h>

#include <algorithm>

namespace EFG::strct {
categoric::VariablesSet EvidenceAware::getHiddenVariables() const {
  categoric::VariablesSet result;
  for (const auto &cluster : state->clusters) {
    for (auto *node : cluster.nodes) {
      result.emplace(node->variable);
    }
  }
  return result;
}

categoric::VariablesSet EvidenceAware::getObservedVariables() const {
  categoric::VariablesSet result;
  for (const auto &[var, val] : state->evidences) {
    result.emplace(var);
  }
  return result;
}

void EvidenceAware::setEvidence(const categoric::VariablePtr &variable,
                                const std::size_t value,
                                const bool observation_should_prexist) {
  if (variable->size() >= value) {
    throw Error{std::to_string(value), " is an invalid evidence for variable ",
                variable->name()};
  }
  auto node_location = find_node(*state, variable);
  if (std::nullopt == node_location) {
    throw Error{variable->name(), " is a non existing variable"};
  }
  EvidenceNodeLocation evidence_location;
  visit(
      *node_location,
      [this](const HiddenNodeLocation &location) {
        for (const auto &[node, connection] :
             location.node->active_connections) {
          location.node->disabled_connections.emplace(
              node, Connection{nullptr, connection.factor});
          node->disabled_connections.emplace(
              location.node, Connection{nullptr, connection.factor});
        }
        // update clusters
        if (1 == location.cluster->nodes.size()) {
          this->state->clusters.erase(location.cluster);
        } else {
          auto nodes = location.cluster->nodes;
          this->state->clusters.erase(location.cluster);
          nodes.erase(location.node);
          auto split_clusters = compute_clusters(nodes);
          for (auto &cluster : split_clusters) {
            this->state->clusters.emplace_back(std::move(cluster));
          }
        }
      },
      [&evidence_location](const EvidenceNodeLocation &location) {
        evidence_location = location;
      });
  for (auto &[node, connection] :
       evidence_location.node->disabled_connections) {
    auto connection_it =
        node->disabled_connections.find(evidence_location.node);
    connection_it->second.message = make_evidence_message(
        connection_it->second.factor, evidence_location.node->variable,
        evidence_location.evidence->second);
    node->merged_contributions.reset();
  }
  resetBelief();
}

namespace {
void re_connect(Node &a, std::set<Node *> &involved_nodes) {
  a.merged_contributions.reset();
  involved_nodes.emplace(&a);
  while (!a.disabled_connections.empty()) {
    auto disabled_it = a.disabled_connections.begin();
    auto *b = disabled_it->first;
    involved_nodes.emplace(b);
    a.active_connections.emplace(
        b, Connection{nullptr, disabled_it->second.factor});
    b->active_connections.emplace(
        &a, Connection{nullptr, disabled_it->second.factor});
    b->merged_contributions.reset();
    a.disabled_connections.erase(disabled_it);
  }
}

void update_clusters(std::vector<HiddenCluster> &clusters,
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

void EvidenceAware::removeEvidence(const categoric::VariablePtr &variable) {
  auto evidence_it = state->evidences.find(variable);
  if (evidence_it == state->evidences.end()) {
    throw Error{variable->name(), " is not an evidence"};
  }
  resetBelief();
  state->evidences.erase(evidence_it);
  std::set<Node *> involved_nodes;
  re_connect(state->nodes[variable], involved_nodes);
  update_clusters(state->clusters, involved_nodes);
}

void EvidenceAware::removeEvidences() {
  if (state->evidences.empty()) {
    return;
  }
  resetBelief();
  std::set<Node *> involved_nodes;
  for (const auto &[var, val] : state->evidences) {
    re_connect(state->nodes[var], involved_nodes);
  }
  state->evidences.clear();
  update_clusters(state->clusters, involved_nodes);
}
} // namespace EFG::strct
