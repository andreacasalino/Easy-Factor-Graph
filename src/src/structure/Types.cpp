/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/Types.h>

#include <algorithm>
#include <limits>

namespace EFG::strct {
namespace {
std::vector<const factor::Immutable *> gather_unaries(Node &subject) {
  std::vector<const factor::Immutable *> unary_factors;
  for (const auto &factor : subject.unary_factors) {
    unary_factors.push_back(factor.get());
  }
  for (auto &[node, connection] : subject.disabled_connections) {
    unary_factors.push_back(connection.message.get());
  }
  return unary_factors;
}
} // namespace

void Node::updateMergedUnaries() {
  std::vector<const factor::Immutable *> unary_factors = gather_unaries(*this);
  if (unary_factors.empty()) {
    merged_unaries.reset(std::make_unique<factor::MergedUnaries>(variable));
    return;
  }
  merged_unaries.reset(std::make_unique<factor::MergedUnaries>(unary_factors));
}

namespace {
Node::Connection *reset(Node::Connection &subject,
                        const factor::ImmutablePtr &factor) {
  subject.message.reset();
  subject.factor = factor;
  return &subject;
}
} // namespace

std::pair<Node::Connection *, Node::Connection *>
Node::activate(Node &a, Node &b, factor::ImmutablePtr factor) {
  a.disabled_connections.erase(&b);
  b.disabled_connections.erase(&a);
  return std::make_pair(reset(a.active_connections[&b], factor),
                        reset(b.active_connections[&a], factor));
}

std::pair<Node::Connection *, Node::Connection *>
Node::disable(Node &a, Node &b, factor::ImmutablePtr factor) {
  if (factor == nullptr) {
    auto it = a.active_connections.find(&b);
    if (it == a.active_connections.end()) {
      throw Error::make("Nodes named: ", a.variable->name(), " and ",
                        b.variable->name(), " are not connected");
    }
    factor = it->second.factor;
  }
  a.active_connections.erase(&b);
  b.active_connections.erase(&a);
  return std::make_pair(reset(a.disabled_connections[&b], factor),
                        reset(b.disabled_connections[&a], factor));
}

bool HiddenCluster::TopologyInfo::canUpdateMessage() const {
  return std::find_if(dependencies.begin(), dependencies.end(),
                      [](const Node::Connection *connection) {
                        return connection->message == nullptr;
                      }) == dependencies.end();
}

std::optional<float>
HiddenCluster::TopologyInfo::updateMessage(PropagationKind kind) {
  if (sender->merged_unaries.empty()) {
    throw Error{"Found node with not updated static dependencies"};
  }
  std::vector<const factor::Immutable *> unary_factors = {
      sender->merged_unaries.get()};
  for (const auto *dep : dependencies) {
    if (nullptr == dep->message) {
      return std::nullopt;
    }
    unary_factors.push_back(dep->message.get());
  }
  factor::MergedUnaries merged_unaries(unary_factors);
  std::unique_ptr<const factor::UnaryFactor> previous_message =
      std::move(connection->message);
  switch (kind) {
  case PropagationKind::SUM:
    connection->message =
        make_message<factor::MessageSUM>(merged_unaries, *connection->factor);
    break;
  case PropagationKind::MAP:
    connection->message =
        make_message<factor::MessageMAP>(merged_unaries, *connection->factor);
    break;
  default:
    throw Error{"Invalid propagation kind"};
  }
  if (nullptr == previous_message) {
    static float MAX_VARIATION = std::numeric_limits<float>::max();
    return MAX_VARIATION;
  }
  return previous_message->diff(*connection->message);
}

void HiddenCluster::updateConnectivity() {
  auto &topology = connectivity.reset(
      std::make_unique<std::vector<HiddenCluster::TopologyInfo>>());
  for (auto *sender : nodes) {
    sender->updateMergedUnaries();
    if (sender->active_connections.empty()) {
      continue;
    }
    std::unordered_set<Node::Connection *> all_dependencies;
    for (auto &[receiver, incoming_connection] : sender->active_connections) {
      all_dependencies.emplace(&incoming_connection);
    }
    for (auto &[receiver, incoming_connection] : sender->active_connections) {
      auto &added = topology.emplace_back();
      added.sender = sender;
      added.connection = &receiver->active_connections.find(sender)->second;
      auto deps = all_dependencies;
      deps.erase(&incoming_connection);
      added.dependencies =
          std::vector<const Node::Connection *>{deps.begin(), deps.end()};
    }
  }
}

namespace {
std::list<std::unordered_set<Node *>>
compute_clusters_(const std::unordered_set<Node *> &nodes) {
  using Cluster = std::unordered_set<Node *>;
  std::list<Cluster> res;
  for (auto *node : nodes) {
    using Iter = std::list<Cluster>::iterator;
    std::unordered_map<const Cluster *, Iter> neighbour;
    for (const auto &[node, _] : node->active_connections) {
      auto it = std::find_if(res.begin(), res.end(),
                             [&node = node](const Cluster &cl) {
                               return cl.find(node) != cl.end();
                             });
      if (it != res.end()) {
        neighbour[&(*it)] = it;
      }
    }
    switch (neighbour.size()) {
    case 0:
      res.emplace_back().emplace(node);
      break;
    case 1:
      neighbour.begin()->second->emplace(node);
      break;
    default: {
      auto &added = res.emplace_back();
      added.emplace(node);
      for (const auto &[_, it] : neighbour) {
        added.insert(it->begin(), it->end());
        res.erase(it);
      }
    } break;
    }
  }
  return res;
}
} // namespace

HiddenClusters compute_clusters(const std::unordered_set<Node *> &nodes) {
  HiddenClusters res;
  for (auto &&set : compute_clusters_(nodes)) {
    res.emplace_back().nodes = std::move(set);
  }
  return res;
}
} // namespace EFG::strct
