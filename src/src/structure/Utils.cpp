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

namespace {
categoric::Group extract_evidence(const categoric::Group &vars,
                                  const categoric::VariablePtr &evidence_var) {
  auto soup = vars.getVariables();
  if (evidence_var == soup.front()) {
    return categoric::Group{soup.back()};
  }
  return categoric::Group{soup.front()};
}

class MarginalizedEvidence : public distribution::Factor {
public:
  MarginalizedEvidence(const distribution::DistributionCnstPtr &binary_factor,
                       const categoric::VariablePtr &evidence_var,
                       const std::size_t evidence)
      : distribution::Factor(
            extract_evidence(binary_factor->getVariables(), evidence_var)) {
    auto &map = getCombinationsMap_();
    std::size_t remaining_pos = 0;
    std::size_t leaving_pos = 1;
    if (getVariables().getVariables().front() == evidence_var) {
      std::swap(remaining_pos, leaving_pos);
    }
    const auto &evaluator = binary_factor->getEvaluator();
    for (const auto &[comb, val] : binary_factor->getCombinationsMap()) {
      const auto &comb_data = comb.data();
      if (comb_data[leaving_pos] == evidence) {
        map.emplace(std::vector<std::size_t>{comb_data[remaining_pos]},
                    evaluator.evaluate(val));
      }
    }
  }
};
} // namespace

distribution::DistributionCnstPtr
marginalized_evidence(const distribution::DistributionCnstPtr &binary_factor,
                      const categoric::VariablePtr &evidence_var,
                      const std::size_t evidence) {
  return std::make_shared<MarginalizedEvidence>(binary_factor, evidence_var,
                                                evidence);
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

namespace {
class UnaryMerged : public distribution::Factor {
public:
  UnaryMerged(
      const std::vector<const distribution::Distribution *> &unary_factors)
      : distribution::Factor(categoric::Group{
            unary_factors.front()->getVariables().getVariables().front()}) {
    auto &map = getCombinationsMap_();
    auto &var = getVariables().getVariables().front();
    float val;
    const auto size = var->size();
    for (std::size_t k = 0; k < size; ++k) {
      categoric::Combination comb({k});
      val = 1.f;
      for (const auto *factor : unary_factors) {
        val *= factor->evaluate(comb);
      }
      map.emplace(k, val);
    }
  }

  UnaryMerged(const categoric::VariablePtr &var)
      : distribution::Factor(categoric::Group{var}) {
    auto &map = getCombinationsMap_();
    const auto size = var->size();
    for (std::size_t k = 0; k < size; ++k) {
      map.emplace(k, 1.f);
    }
  }
};
} // namespace

void update_merged_unaries(Node &subject) {
  std::vector<const distribution::Distribution *> unary_factors;
  for (const auto &factor : subject.unary_factors) {
    unary_factors.push_back(factor.get());
  }
  for (auto &[node, connection] : subject.disabled_connections) {
    unary_factors.push_back(connection.message.get());
  }
  if (unary_factors.empty()) {
    subject.merged_unaries = std::make_unique<UnaryMerged>(subject.variable);
    return;
  }
  subject.merged_unaries = std::make_unique<UnaryMerged>(unary_factors);
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
    for (const auto &[receiver, incoming_connection] :
         sender->active_connections) {
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
  for (const auto *dep : subject.dependencies) {
    if (nullptr == dep->message) {
      return false;
    }
  }
  return true;
}

namespace {
class MessageSUM : public distribution::Factor {
public:
  MessageSUM(const UnaryMerged &messages,
             const distribution::Distribution &binary_factor);
};

////////// remember to normalize messages

class MessageMAP : public distribution::Factor {
public:
  MessageMAP(const UnaryMerged &messages,
             const distribution::Distribution &binary_factor);
};

float diff(const distribution::Distribution &a,
           const distribution::Distribution &b) {
  auto &map_a = a.getCombinationsMap();
  auto &map_b = b.getCombinationsMap();
  auto map_a_it = map_a.begin();
  float result = 0;
  int diff;
  for (auto map_b_it = map_b.begin(); map_b_it != map_b.end();
       ++map_b_it, ++map_a_it) {
    diff = static_cast<int>(map_a_it->first.data().front());
    diff -= static_cast<int>(map_b_it->first.data().front());
    result += abs(diff);
  }
  return result;
}
} // namespace

MessageVariation update_message(ConnectionAndDependencies &subject,
                                const PropagationKind &kind) {
  if (nullptr == subject.sender->merged_unaries) {
    update_merged_unaries(*subject.sender);
  }
  std::vector<const distribution::Distribution *> unary_factors = {
      subject.sender->merged_unaries.get()};
  for (const auto *dep : subject.dependencies) {
    unary_factors.push_back(dep->message.get());
  }
  UnaryMerged unary_factors_merged(unary_factors);
  auto previous_message = subject.connection->message;
  switch (kind) {
  case SUM:
    subject.connection->message = std::make_shared<MessageSUM>(
        unary_factors_merged, *subject.connection->factor);
    break;
  case MAP:
    subject.connection->message = std::make_shared<MessageMAP>(
        unary_factors_merged, *subject.connection->factor);
    break;
  }
  if (nullptr == previous_message) {
    return MAX_VARIATION;
  }
  return diff(*previous_message, *subject.connection->message);
}
} // namespace EFG::strct

namespace EFG::distribution {} // namespace EFG::distribution
