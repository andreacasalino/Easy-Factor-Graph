/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/BaselineLoopyPropagator.h>
#include <EasyFactorGraph/structure/bases/BeliefAware.h>

namespace EFG::strct {
BeliefAware::BeliefAware() {
  loopy_propagator = std::make_unique<BaselineLoopyPropagator>();
}

void BeliefAware::setLoopyPropagationStrategy(
    LoopyBeliefPropagationStrategyPtr strategy) {
  if (nullptr == strategy) {
    throw Error{"trying to set a null propagator"};
  }
  loopy_propagator = std::move(strategy);
}

bool BeliefAware::wouldNeedPropagation(PropagationKind kind) const {
  return (!lastPropagation.has_value()) ||
         (lastPropagation->propagation_kind_done != kind);
}

namespace {
void reset_messages(const HiddenClusters &clusters) {
  for (const auto &cluster : clusters) {
    for (auto *node : cluster.nodes) {
      for (auto &[connected_node, connection] : node->active_connections) {
        connection.message.reset();
      }
    }
  }
}

std::list<HiddenCluster::TopologyInfo *>
pack_messages(std::vector<HiddenCluster::TopologyInfo> &conn) {
  std::list<HiddenCluster::TopologyInfo *> res;
  for (auto &el : conn) {
    res.push_back(&el);
  }
  return res;
}

bool message_passing(HiddenCluster &cluster, const PropagationKind &kind,
                     Pool &pool) {
  std::list<HiddenCluster::TopologyInfo *> leftToCompute =
      pack_messages(*cluster.connectivity.get());
  while (!leftToCompute.empty()) {
    Tasks to_process;
    auto it = leftToCompute.begin();
    while (it != leftToCompute.end()) {
      if ((*it)->canUpdateMessage()) {
        to_process.emplace_back([task = *it, kind](const std::size_t) {
          task->updateMessage(kind);
        });
        it = leftToCompute.erase(it);
      } else {
        ++it;
      }
    }
    if (to_process.empty()) {
      return false;
    }
    pool.parallelFor(to_process);
  }
  return true;
}
} // namespace

void BeliefAware::propagateBelief(PropagationKind kind) {
  if (!wouldNeedPropagation(kind)) {
    return;
  }

  auto &clusters = stateMutable().clusters;
  auto &pool = getPool();
  reset_messages(clusters);

  PropagationResult result;
  result.was_completed = true;
  result.propagation_kind_done = kind;
  for (auto &cluster : clusters) {
    if (cluster.connectivity.empty()) {
      cluster.updateConnectivity();
    } else {
      for (auto *node : cluster.nodes) {
        if (node->merged_unaries.empty()) {
          node->updateMergedUnaries();
        }
      }
    }
    if (message_passing(cluster, kind, pool)) {
      auto &cluster_info = result.structures.emplace_back();
      cluster_info.tree_or_loopy_graph = true;
      cluster_info.size = cluster.nodes.size();
      continue;
    }

    auto &cluster_info = result.structures.emplace_back();
    cluster_info.tree_or_loopy_graph = false;
    cluster_info.size = cluster.nodes.size();

    if (!loopy_propagator->propagateBelief(cluster, kind, context, pool)) {
      result.was_completed = false;
    }
  }
  lastPropagation = result;
}
} // namespace EFG::strct
