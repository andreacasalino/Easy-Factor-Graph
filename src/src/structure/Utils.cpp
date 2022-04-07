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
void visit_location(
    const NodeLocation &to_visit,
    std::function<void(const HiddenNodeLocation &)> hidden_case,
    std::function<void(const EvidenceNodeLocation &)> evidence_case) {
  const auto *as_hidden = std::get_if<HiddenNodeLocation>(&to_visit);
  if (nullptr != as_hidden) {
    hidden_case(*as_hidden);
    return;
  }
  const auto *as_evidence = std::get_if<EvidenceNodeLocation>(&to_visit);
  if (nullptr != as_evidence) {
    evidence_case(*as_evidence);
    return;
  }
  throw Error{"Empty location can't be visited"};
}

namespace {
using ClustersRaw = std::list<std::set<Node *>>;

struct ClustersRawIteratorComparer {
  bool operator()(const ClustersRaw::const_iterator &a,
                  const ClustersRaw::const_iterator &b) const {
    return &(*a) < &(*b);
  }
};

using ClustersRawIterators =
    std::set<ClustersRaw::const_iterator, ClustersRawIteratorComparer>;

ClustersRawIterators find_connections(const ClustersRaw &clusters,
                                      Node &to_explore) {
  ClustersRawIterators result;
  for (const auto &[connected_node, connection] :
       to_explore.active_connections) {
    auto existing_cluster = std::find_if(
        clusters.begin(), clusters.end(),
        [&connected_node = connected_node](const std::set<Node *> &cluster) {
          return cluster.find(connected_node) != cluster.end();
        });
    if (existing_cluster != clusters.end()) {
      result.emplace(existing_cluster);
    }
  }
  return result;
}

ClustersRaw compute_clusters_sets(const std::set<Node *> &nodes) {
  ClustersRaw result;
  for (auto *to_visit : nodes) {
    auto connections = find_connections(result, *to_visit);
    std::set<Node *> *recipient = nullptr;
    if (connections.empty()) {
      recipient = &result.emplace_back();
    } else {
      std::set<Node *> new_cluster;
      for (const auto &existing_cluster_it : connections) {
        new_cluster.insert(existing_cluster_it->begin(),
                           existing_cluster_it->end());
        result.erase(existing_cluster_it);
      }
      recipient = &result.emplace_back(std::move(new_cluster));
    }
    recipient->emplace(to_visit);
  }
  return result;
}
} // namespace

HiddenClusters compute_clusters(const std::set<Node *> &nodes) {
  auto sets = compute_clusters_sets(nodes);
  HiddenClusters result;
  for (auto &set : sets) {
    auto &added = result.emplace_back();
    added.nodes = std::move(set);
  }
  return result;
}

void activate_connection(Node &a, Node &b,
                         const distribution::DistributionCnstPtr &factor) {
  a.active_connections.erase(&b);
  b.active_connections.erase(&a);
  a.active_connections.emplace(&b, std::make_unique<Connection>())
      .first->second->factor = factor;
  b.active_connections.emplace(&a, std::make_unique<Connection>())
      .first->second->factor = factor;
}

void disable_connection(Node &a, Node &b,
                        const distribution::DistributionCnstPtr &factor) {
  a.disabled_connections.erase(&b);
  b.disabled_connections.erase(&a);
  a.disabled_connections.emplace(&b, std::make_unique<Connection>())
      .first->second->factor = factor;
  b.disabled_connections.emplace(&a, std::make_unique<Connection>())
      .first->second->factor = factor;
}

std::vector<const distribution::Distribution *> gather_unaries(Node &subject) {
  std::vector<const distribution::Distribution *> unary_factors;
  for (const auto &factor : subject.unary_factors) {
    unary_factors.push_back(factor.get());
  }
  for (auto &[node, connection] : subject.disabled_connections) {
    unary_factors.push_back(connection->message.get());
  }
  return unary_factors;
}

void update_merged_unaries(Node &subject) {
  std::vector<const distribution::Distribution *> unary_factors =
      gather_unaries(subject);
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
      all_dependencies.emplace(incoming_connection.get());
    }
    for (auto &[receiver, incoming_connection] : sender->active_connections) {
      auto deps = all_dependencies;
      deps.erase(incoming_connection.get());
      auto &added = connectivity.emplace_back();
      added.sender = sender;
      added.dependencies =
          std::vector<const Connection *>{deps.begin(), deps.end()};
      added.connection = receiver->active_connections[sender].get();
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
