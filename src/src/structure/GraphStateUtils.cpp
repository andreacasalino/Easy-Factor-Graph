/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include "GraphStateUtils.h"
#include "BeliefPropagationUtils.h"
#include <EasyFactorGraph/distribution/Factor.h>

namespace EFG::strct {
NodeInfo find_node(GraphState &state, Node &to_find) {
  auto hidden_clusters_it =
      std::find_if(state.hidden_clusters.begin(), state.hidden_clusters.end(),
                   [&to_find](HiddenCluster &cluster) {
                     return cluster.nodes.find(&to_find) != cluster.nodes.end();
                   });
  if (hidden_clusters_it != state.hidden_clusters.end()) {
    return hidden_clusters_it;
  }
  return state.evidences.find(to_find.variable);
}

void disable_connection(Node &nodeA, Node &nodeB) {
  auto distribution = nodeA.activeConnections.find(&nodeB)->second.factor;
  nodeA.activeConnections.erase(&nodeB);
  nodeB.activeConnections.erase(&nodeA);
  nodeA.disabledConnections[&nodeB] = Connection{distribution, nullptr};
  nodeA.disabledConnections[&nodeA] = Connection{distribution, nullptr};
}

void enable_connection(Node &nodeA, Node &nodeB) {
  auto distribution = nodeA.disabledConnections.find(&nodeB)->second.factor;
  nodeA.disabledConnections.erase(&nodeB);
  nodeB.disabledConnections.erase(&nodeA);
  nodeA.activeConnections[&nodeB] = Connection{distribution, nullptr};
  nodeA.activeConnections[&nodeA] = Connection{distribution, nullptr};
}

std::unique_ptr<const distribution::Distribution>
make_evidence_message(const distribution::DistributionCnstPtr &binary_factor,
                      const categoric::VariablePtr &evidence_var,
                      const std::size_t evidence) {
  distribution::CombinationRawValuesMapPtr map;
  std::size_t leaving = 1;
  std::size_t remaining = 0;
  if (evidence_var == binary_factor->getVariables().getVariables().front()) {
    std::swap(leaving, remaining);
  }
  const auto &evaluator = binary_factor->getEvaluator();
  for (const auto &[comb, val] : binary_factor->getCombinationsMap()) {
    if (comb.data()[leaving] == evidence) {
      map->emplace(std::vector<std::size_t>{comb.data()[remaining]},
                   evaluator.evaluate(val));
    }
  }

  categoric::VariablePtr hidden_var;
  if (binary_factor->getVariables().getVariables().front() == evidence_var) {
    hidden_var = binary_factor->getVariables().getVariables().back();
  } else {
    hidden_var = binary_factor->getVariables().getVariables().front();
  }

  class EvidenceMessage : public distribution::Factor {
  public:
    EvidenceMessage(const categoric::VariablePtr &var,
                    const distribution::CombinationRawValuesMapPtr &map)
        : distribution::Factor(categoric::Group{var}, map){};
  };

  return std::make_unique<EvidenceMessage>(hidden_var, map);
}

namespace {
using NodeSet = std::set<Node *>;

std::vector<NodeSet>::iterator find_set(std::vector<NodeSet> &clusters,
                                        Node &to_explore) {
  return std::find_if(clusters.begin(), clusters.end(),
                      [&to_explore](const NodeSet &cluster) {
                        return cluster.find(&to_explore) != cluster.end();
                      });
}

struct ConnectionsResult {
  std::vector<Node *> not_connected;
  std::vector<std::vector<NodeSet>::iterator> existing_clusters;
};
ConnectionsResult find_connections(std::vector<NodeSet> &clusters,
                                   Node &to_explore) {
  ConnectionsResult result;
  for (const auto &[node, connection] : to_explore.activeConnections) {
    auto existing_cluster = find_set(clusters, *node);
    if (existing_cluster == clusters.end()) {
      result.not_connected.push_back(node);
    } else {
      result.existing_clusters.push_back(existing_cluster);
    }
  }
  return result;
}

std::vector<NodeSet> compute_clusters_sets(const std::set<Node *> &nodes) {
  if (nodes.empty()) {
    return {};
  }
  std::vector<std::set<Node *>> result;
  auto open = nodes;
  while (!open.empty()) {
    auto *to_visit = *open.begin();
    auto connections = find_connections(result, *to_visit);
    open.erase(to_visit);
    for (auto *node : connections.not_connected) {
      open.erase(node);
    }
    std::set<Node *> *recipient = nullptr;
    if (connections.existing_clusters.empty()) {
      recipient = &result.emplace_back();
    } else {
      recipient = &(*connections.existing_clusters.front());
      for (std::size_t k = 1; k < connections.existing_clusters.size(); ++k) {
        recipient->insert(connections.existing_clusters[k]->begin(),
                          connections.existing_clusters[k]->end());
        result.erase(connections.existing_clusters[k]);
      }
    }
    recipient->emplace(to_visit);
    recipient->insert(connections.not_connected.begin(),
                      connections.not_connected.end());
  }
  return result;
}

template <typename T> void erase(std::vector<T> &subject, const T &to_remove) {
  subject.erase(std::find(subject.begin((), subject.end(), to_remove)));
}

std::vector<MessageAndDependencies>
compute_dependencies(const std::set<Node *> &cluster) {
  std::vector<MessageAndDependencies> result;
  for (auto *sender : cluster) {
    if (sender->activeConnections.empty()) {
      continue;
    }
    MessageAndDependencies all_deps;
    all_deps.sender = sender->variable;
    auto static_factors = sender->unaryFactors;
    for (const auto &[connected_node, connection] :
         sender->disabledConnections) {
      static_factors.push_back(connection.message2ThisNode);
    }
    all_deps.static_merged_dependencies = merge_unary_factors(static_factors);
    for (const auto &[connected_node, connection] : sender->activeConnections) {
      all_deps.dynamic_dependencies.push_back(&connection.message2ThisNode);
    }
    for (const auto &[connected_node, connection] : sender->activeConnections) {
      result.push_back(all_deps);
      result.back().connection =
          &connected_node->activeConnections.find(sender)->second;
      erase(result.back().dynamic_dependencies, &connection.message2ThisNode);
    }
  }
  return result;
}
} // namespace

std::vector<HiddenCluster> compute_clusters(const std::set<Node *> &nodes) {
  auto sets = compute_clusters_sets(nodes);
  std::vector<HiddenCluster> result;
  result.reserve(sets.size());
  for (auto &set : sets) {
    auto &added = result.emplace_back();
    added.nodes = std::move(set);
    added.messages = compute_dependencies(added.nodes);
  }
  return result;
}
} // namespace EFG::strct
