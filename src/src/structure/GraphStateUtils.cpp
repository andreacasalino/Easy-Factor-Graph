/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include "GraphStateUtils.h"
#include "BeliefPropagationUtils.h"
#include <EasyFactorGraph/distribution/Factor.h>

#include <algorithm>

namespace EFG::strct {
std::vector<HiddenCluster>::iterator
find_cluster(GraphState &state, const categoric::VariablePtr &variable) {
  auto nodes_it = state.nodes.find(variable);
  if (nodes_it == state.nodes.end()) {
    return state.clusters.end();
  }
  auto *node = &nodes_it->second;
  return std::find_if(state.clusters.begin(), state.clusters.end(),
                      [&node](const HiddenCluster &cluster) {
                        return cluster.nodes.find(node) != cluster.nodes.end();
                      });
}

std::optional<NodeLocation> find_node(GraphState &state,
                                      const categoric::VariablePtr &variable) {
  auto evidences_it = state.evidences.find(variable);
  if (evidences_it != state.evidences.end()) {
    return EvidenceNodeLocation{evidences_it, &state.nodes[variable]};
  }
  auto clusters_it = find_cluster(state, variable);
  if (clusters_it != state.clusters.end()) {
    return HiddenNodeLocation{clusters_it, &state.nodes[variable]};
  }
  return std::nullopt;
}

void visit(const NodeLocation &to_visit,
           std::function<void(const HiddenNodeLocation &)> hidden_case,
           std::function<void(const EvidenceNodeLocation &)> evidence_case) {
  struct Visitor {
    std::function<void(const HiddenNodeLocation &)> hidden_case;
    std::function<void(const EvidenceNodeLocation &)> evidence_case;

    void operator()(const HiddenNodeLocation &info) const {
      hidden_case(info);
    };
    void operator()(const EvidenceNodeLocation &info) const {
      evidence_case(info);
    };
  };
  std::visit(Visitor{hidden_case, evidence_case}, to_visit);
}

distribution::DistributionCnstPtr
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

  return std::make_shared<EvidenceMessage>(hidden_var, map);
}

namespace {
std::vector<std::set<Node *>>::const_iterator
find_set(const std::vector<std::set<Node *>> &clusters, Node &to_find) {
  return std::find_if(clusters.begin(), clusters.end(),
                      [&to_find](const std::set<Node *> &cluster) {
                        return cluster.find(&to_find) != cluster.end();
                      });
}

struct ConnectionsResult {
  std::vector<Node *> not_connected;
  std::set<std::size_t> existing_clusters;
};
ConnectionsResult
find_connections(const std::vector<std::set<Node *>> &clusters,
                 Node &to_explore) {
  ConnectionsResult result;
  for (const auto &[node, connection] : to_explore.active_connections) {
    auto existing_cluster = find_set(clusters, *node);
    if (existing_cluster == clusters.end()) {
      result.not_connected.push_back(node);
    } else {
      result.existing_clusters.emplace(
          std::distance(clusters.begin(), existing_cluster));
    }
  }
  return result;
}

template <typename T>
void erase(std::vector<T> &subject, const std::size_t pos) {
  auto it = subject.begin();
  std::advance(it, pos);
  subject.erase(it);
}

std::vector<std::set<Node *>>
compute_clusters_sets(const std::set<Node *> &nodes) {
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
      auto existing_clusters_it = connections.existing_clusters.rbegin();
      auto existing_clusters_end = connections.existing_clusters.rend();
      --existing_clusters_end;
      recipient = &result[*existing_clusters_end];
      for (existing_clusters_it; existing_clusters_it != existing_clusters_end;
           ++existing_clusters_it) {
        auto &cluster = result[*existing_clusters_it];
        recipient->insert(cluster.begin(), cluster.end());
        erase(result, *existing_clusters_it);
      }
    }
    recipient->emplace(to_visit);
    recipient->insert(connections.not_connected.begin(),
                      connections.not_connected.end());
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
  }
  return result;
}
} // namespace EFG::strct
