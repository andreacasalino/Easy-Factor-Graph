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
#include <mutex>

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

std::list<ConnectionAndDependencies *>
pack_all_tasks(std::vector<ConnectionAndDependencies> &tasks) {
  std::list<ConnectionAndDependencies *> result;
  for (auto &task : tasks) {
    result.push_back(&task);
  }
  return result;
}

bool message_passing(HiddenCluster &cluster, const PropagationKind &kind,
                     Pool &pool) {
  auto open = pack_all_tasks(*cluster.connectivity);
  while (!open.empty()) {
    Tasks to_process;
    auto open_it = open.begin();
    while (open_it != open.end()) {
      if (can_update_message(**open_it)) {
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

class MaxVariation {
public:
  MaxVariation() = default;

  void reset() {
    std::scoped_lock lock(max_variation_mtx);
    max_variation = 0;
  }
  void update(const MessageVariation &value) {
    std::scoped_lock lock(max_variation_mtx);
    if (value > max_variation) {
      max_variation = value;
    }
  }

  MessageVariation get() const {
    std::scoped_lock lock(max_variation_mtx);
    return max_variation;
  }

private:
  MessageVariation max_variation = 0;
  std::mutex max_variation_mtx;
};

bool has_locked_dependency(const ConnectionAndDependencies &subject,
                           const std::set<const Connection *> &locked) {
  for (const auto *dep : subject.dependencies) {
    if (locked.find(dep) != locked.end()) {
      return true;
    }
  }
  return false;
}

std::vector<Tasks> compute_loopy_order(HiddenCluster &cluster,
                                       const PropagationKind &kind,
                                       MaxVariation &max_variation,
                                       const bool is_multithreaded) {
  auto make_task = [&kind, &max_variation](ConnectionAndDependencies &subject) {
    return [&task = subject, &kind, &max_variation]() {
      max_variation.update(*update_message(task, kind));
    };
  };

  std::vector<Tasks> result;
  auto open = pack_all_tasks(*cluster.connectivity);
  if (is_multithreaded) {
    while (!open.empty()) {
      std::set<const Connection *> locked;
      auto &new_tasks = result.emplace_back();
      auto open_it = open.begin();
      while (open_it != open.end()) {
        if (!has_locked_dependency(**open_it, locked)) {
          locked.emplace((*open_it)->connection);
          new_tasks.emplace_back(make_task(**open_it));
          open_it = open.erase(open_it);
        } else {
          ++open_it;
        }
      }
    }
  } else {
    auto &new_tasks = result.emplace_back();
    new_tasks.reserve(open.size());
    for (auto *task : open) {
      new_tasks.emplace_back(make_task(*task));
    }
  }
  return result;
}

static const MessageVariation DEFAULT_VARIATION_TOLLERANCE = 1e-3;

struct LoopyPropagationContext {
  PropagationKind kind;
  std::size_t max_iter;
  MessageVariation variation_tollerance;
};
bool loopy_propagation(HiddenCluster &cluster,
                       const LoopyPropagationContext &ctx, Pool &pool) {
  auto open = pack_all_tasks(*cluster.connectivity);
  // set message to ones
  for (const auto *task : open) {
    task->connection->message = make_unary(task->sender->variable);
  }
  MaxVariation max_variation;
  auto order =
      compute_loopy_order(cluster, ctx.kind, max_variation, 1 < pool.size());
  for (std::size_t iter = 0; iter < ctx.max_iter; ++iter) {
    max_variation.reset();
    for (const auto &tasks : order) {
      pool.parallelFor(tasks);
    }
    if (0 == max_variation.get()) {
      return true;
    }
  }
  return false;
}
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
    if (!loopy_propagation(
            cluster,
            LoopyPropagationContext{
                kind, getPropagationContext().max_iterations_loopy_propagation,
                DEFAULT_VARIATION_TOLLERANCE},
            getPool())) {
      result.was_completed = false;
    }
  }
  return result;
}
} // namespace EFG::strct
