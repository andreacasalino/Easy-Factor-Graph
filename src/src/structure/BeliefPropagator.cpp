/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/structure/BeliefPropagator.h>

#include "Utils.h"

#include <algorithm>
#include <list>
#include <omp.h>

namespace EFG::strct {
namespace {
void resetMessages(const std::vector<HiddenCluster> &clusters) {
  for (const auto &cluster : clusters) {
    for (auto *node : cluster.nodes) {
      for (auto &[connected_node, connection] : node->active_connections) {
        connection.message.reset();
      }
    }
  }
}

bool has_locked_dependency(const ConnectionAndDependencies &subject,
                           const std::set<const Connection *> &locked) {
  for (const auto *dep : subject.dependencies) {
    if (locked.find(dep) != locked.end()) {
      return true;
    }
  }
  return false;
}

bool message_passing(HiddenCluster &cluster, const PropagationKind &kind,
                     Pool &pool) {
  std::list<ConnectionAndDependencies *> open;
  for (auto &task : *cluster.connectivity) {
    open.push_back(&task);
  }
  while (!open.empty()) {
    std::set<const Connection *> locked;
    Tasks to_process;
    auto open_it = open.begin();
    while (open_it != open.end()) {
      if (can_update_message(**open_it) &&
          !has_locked_dependency(**open_it, locked)) {
        locked.emplace((*open_it)->connection);
        open_it = open.erase(open_it);
        to_process.emplace_back(
            [&task = **open_it, &kind]() { update_message(task, kind); });
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

bool loopy_propagation(HiddenCluster &cluster, const PropagationKind &kind,
                       Pool &pool);
} // namespace

PropagationResult
BeliefPropagator::propagateBelief(const PropagationKind &kind) {
  if (hasPropagationResult() &&
      (kind == getLastPropagationResult().propagation_kind_done)) {
    return;
  }
  auto &clusters = getClusters_();
  resetMessages(clusters);
  PropagationResult result;
  result.was_completed = true;
  result.propagation_kind_done = kind;
  for (auto &cluster : clusters) {
    if (nullptr == cluster.connectivity) {
      update_connectivity(cluster);
    }
    if (message_passing(cluster, kind, getPool())) {
      continue;
    }
    if (!loopy_propagation(cluster, kind, getPool())) {
      result.was_completed = false;
    }
  }
  return result;
}
} // namespace EFG::strct
