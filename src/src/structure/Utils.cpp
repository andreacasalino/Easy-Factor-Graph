/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/distribution/Factor.h>

#include "Utils.h"

#include <algorithm>
#include <math.h>

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

void visit_location(
    const NodeLocation &to_visit,
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

namespace {
struct ConnectionsResult {
  std::vector<Node *> not_connected;
  std::set<std::size_t> existing_clusters;
};
ConnectionsResult
find_connections(const std::vector<std::set<Node *>> &clusters,
                 Node &to_explore) {
  ConnectionsResult result;
  for (const auto &[node, connection] : to_explore.active_connections) {
    auto existing_cluster =
        std::find_if(clusters.begin(), clusters.end(),
                     [&to_explore](const std::set<Node *> &cluster) {
                       return cluster.find(&to_explore) != cluster.end();
                     });
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
      std::set<Node *> new_cluster;
      for (auto clusters_it = connections.existing_clusters.rbegin();
           clusters_it != connections.existing_clusters.rend(); ++clusters_it) {
        new_cluster.insert(result[*clusters_it].begin(),
                           result[*clusters_it].end());
        erase(result, *clusters_it);
      }
      recipient = &result.emplace_back(std::move(new_cluster));
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

void update_merged_unaries(Node &subject) {
  std::vector<const distribution::Distribution *> unary_factors;
  for (const auto &factor : subject.unary_factors) {
    unary_factors.push_back(factor.get());
  }
  for (auto &[node, connection] : subject.disabled_connections) {
    unary_factors.push_back(connection.message.get());
  }
  if (unary_factors.empty()) {
    subject.merged_unaries = make_unary(subject.variable);
    return;
  }
  subject.merged_unaries = make_unary(unary_factors);
}

void update_connectivity(HiddenCluster &subject) {
  subject.connectivity =
      std::make_unique<std::vector<ConnectionAndDependencies>>();
  auto &connectivity = *subject.connectivity;
  for (auto *sender : subject.nodes) {
    if (sender->active_connections.empty()) {
      continue;
    }
    std::set<Connection *> all_dependencies;
    for (auto &[receiver, incoming_connection] : sender->active_connections) {
      all_dependencies.emplace(&incoming_connection);
    }
    for (auto &[receiver, incoming_connection] : sender->active_connections) {
      auto deps = all_dependencies;
      deps.erase(&incoming_connection);
      auto &added = connectivity.emplace_back();
      added.sender = sender;
      added.dependencies =
          std::vector<const Connection *>{deps.begin(), deps.end()};
      added.connection = &receiver->active_connections[sender];
    }
  }
}

bool can_update_message(const ConnectionAndDependencies &subject) {
  return std::find_if(subject.dependencies.begin(), subject.dependencies.end(),
                      [](const Connection *connection) {
                        return connection->message == nullptr;
                      }) == subject.dependencies.end();
}

namespace {
float diff(const distribution::UnaryFactor &a,
           const distribution::UnaryFactor &b) {
  auto &map_a = a.getCombinationsMap();
  auto &map_b = b.getCombinationsMap();
  auto map_a_it = map_a.begin();
  float result = 0;
  int diff;
  for (auto map_b_it = map_b.begin(); map_b_it != map_b.end();
       ++map_b_it, ++map_a_it) {
    diff = static_cast<int>(map_a_it->second);
    diff -= static_cast<int>(map_b_it->second);
    result += abs(diff);
  }
  return result;
}
} // namespace

std::optional<MessageVariation>
update_message(ConnectionAndDependencies &subject,
               const PropagationKind &kind) {
  if (nullptr == subject.sender->merged_unaries) {
    update_merged_unaries(*subject.sender);
  }
  std::vector<const distribution::Distribution *> unary_factors = {
      subject.sender->merged_unaries.get()};
  for (const auto *dep : subject.dependencies) {
    if (nullptr == dep->message) {
      return std::nullopt;
    }
    unary_factors.push_back(dep->message.get());
  }
  distribution::UnaryFactor merged_unaries(unary_factors);
  std::unique_ptr<const distribution::UnaryFactor> previous_message =
      std::move(subject.connection->message);
  switch (kind) {
  case SUM:
    subject.connection->message = make_message<distribution::MessageSUM>(
        merged_unaries, *subject.connection->factor);
    break;
  case MAP:
    subject.connection->message = make_message<distribution::MessageMAP>(
        merged_unaries, *subject.connection->factor);
    break;
  }
  if (nullptr == previous_message) {
    return MAX_VARIATION;
  }
  return diff(*previous_message, *subject.connection->message);
}
} // namespace EFG::strct
