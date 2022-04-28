/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/BaselineLoopyPropagator.h>
#include <EasyFactorGraph/structure/components/BeliefAware.h>

#include "../Utils.h"

namespace EFG::strct {
BeliefAware::BeliefAware() {
  loopy_propagator = std::make_unique<BaselineLoopyPropagator>();
}

void BeliefAware::setLoopyPropagationStrategy(
    LoopyBeliefPropagationStrategyPtr strategy) {
  if (nullptr == strategy) {
    throw Error{"trying ot set null propagator"};
  }
  loopy_propagator = std::move(strategy);
}

bool BeliefAware::wouldNeedPropagation(const PropagationKind &kind) const {
  return (lastPropagation == nullptr) ||
         (lastPropagation->propagation_kind_done != kind);
}

namespace {
void resetMessages(const HiddenClusters &clusters) {
  for (const auto &cluster : clusters) {
    for (auto *node : cluster.nodes) {
      for (auto &[connected_node, connection] : node->active_connections) {
        connection->message.reset();
      }
    }
  }
}

bool message_passing(HiddenCluster &cluster, const PropagationKind &kind,
                     Pool &pool) {
  auto open = pack_all_tasks(*cluster.connectivity.get());
  while (!open.empty()) {
    Tasks to_process;
    auto open_it = open.begin();
    while (open_it != open.end()) {
      if (can_update_message(**open_it)) {
        to_process.emplace_back([task = *open_it, &kind](const std::size_t) {
          update_message(*task, kind);
        });
        open_it = open.erase(open_it);
      } else {
        ++open_it;
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

void BeliefAware::propagateBelief(const PropagationKind &kind) {
  if (!wouldNeedPropagation(kind)) {
    return;
  }

  auto &clusters = getState_().clusters;
  auto &pool = getPool();
  resetMessages(clusters);

  PropagationResult result;
  result.was_completed = true;
  result.propagation_kind_done = kind;
  for (auto &cluster : clusters) {
    if (cluster.connectivity.empty()) {
      update_connectivity(cluster);
    } else {
      for (auto *node : cluster.nodes) {
        if (node->merged_unaries.empty()) {
          update_merged_unaries(*node);
        }
      }
    }
    if (message_passing(cluster, kind, pool)) {
      auto &cluster_info = result.structure_found.emplace_back();
      cluster_info.tree_or_loopy_graph = true;
      cluster_info.size = cluster.nodes.size();
      continue;
    }

    auto &cluster_info = result.structure_found.emplace_back();
    cluster_info.tree_or_loopy_graph = false;
    cluster_info.size = cluster.nodes.size();

    if (!loopy_propagator->propagateBelief(cluster, kind, context, pool)) {
      result.was_completed = false;
    }
  }
  lastPropagation = std::make_unique<PropagationResult>(result);
}
} // namespace EFG::strct
