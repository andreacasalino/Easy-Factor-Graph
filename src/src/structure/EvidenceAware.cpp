/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/EvidenceAware.h>

#include "Utils.h"

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
      [this, &observation_should_prexists](const HiddenNodeLocation &location) {
        auto *node = location.node;
        if (observation_should_prexist) {
          throw Error{"Variable ", location.node->variable->name(),
                      " can't be set as an evidence"};
        }
        for (const auto &[connected_node, connection] :
             node->active_connections) {
          node->disabled_connections.emplace(
              connected_node, Connection{nullptr, connection.factor});
          connected_node->disabled_connections.emplace(
              node, Connection{nullptr, connection.factor});
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
  auto *node = evidence_location.node;
  for (auto &[connected_node, connection] : node->disabled_connections) {
    auto connection_it =
        connected_node->disabled_connections.find(evidence_location.node);
    connection_it->second.message = marginalized_evidence(
        connection_it->second.factor, evidence_location.node->variable,
        evidence_location.evidence->second);
    connected_node->merged_unaries.reset();
  }
  resetBelief();
}

namespace {
void re_connect(Node &a, std::set<Node *> &involved_nodes) {
  a.merged_unaries.reset();
  involved_nodes.emplace(&a);
  for (const auto &[connected_node, connection] : a.disabled_connections) {
    involved_nodes.emplace(connected_node);
    a.active_connections.emplace(connected_node,
                                 Connection{nullptr, connection.factor});
    connected_node->active_connections.emplace(
        &a, Connection{nullptr, connection.factor});
    connected_node->disabled_connections.erase(&a);
    connected_node->merged_unaries.reset();
  }
  a.disabled_connections.clear();
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
