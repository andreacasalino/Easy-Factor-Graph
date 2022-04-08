/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/structure/BaselineBeliefPropagator.h>

#include "Utils.h"

#include <algorithm>
#include <list>

namespace EFG::strct {
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

bool has_locked_dependency(const ConnectionAndDependencies &subject,
                           const std::set<const Connection *> &locked) {
  for (const auto *dep : subject.dependencies) {
    if (locked.find(dep) != locked.end()) {
      return true;
    }
  }
  return false;
}

std::vector<Tasks>
compute_loopy_order(HiddenCluster &cluster, const PropagationKind &kind,
                    std::vector<MessageVariation> &variations) {
  auto make_task = [&kind, &variations](ConnectionAndDependencies &subject) {
    return [&task = subject, &kind, &variations](const std::size_t th_id) {
      auto &variation = variations[th_id];
      float candidate = *update_message(task, kind);
      if (candidate > variation) {
        variation = candidate;
      }
    };
  };

  std::vector<Tasks> result;
  auto open = pack_all_tasks(*cluster.connectivity.get());
  if (variations.size() > 1) {
    // multithreaded computation
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

MessageVariation
max_variation(const std::vector<MessageVariation> &variations) {
  MessageVariation result = variations.front();
  for (const auto &variation : variations) {
    if (variation > result) {
      result = variation;
    }
  }
  return result;
}

struct LoopyPropagationContext {
  PropagationKind kind;
  std::size_t max_iter;
  MessageVariation variation_tollerance;
};
bool loopy_propagation(HiddenCluster &cluster,
                       const LoopyPropagationContext &ctx, Pool &pool) {
  auto open = pack_all_tasks(*cluster.connectivity.get());
  // set message to ones
  for (const auto *task : open) {
    task->connection->message = make_unary(task->sender->variable);
  }
  std::vector<MessageVariation> variations;
  variations.resize(pool.size());
  auto order = compute_loopy_order(cluster, ctx.kind, variations);
  for (std::size_t iter = 0; iter < ctx.max_iter; ++iter) {
    for (auto &variation : variations) {
      variation = 0;
    }
    for (const auto &tasks : order) {
      pool.parallelFor(tasks);
    }
    if (0 == max_variation(variations)) {
      return true;
    }
  }
  return false;
}
} // namespace

PropagationResult BaselineBeliefPropagator::propagateBelief(
    HiddenClusters &subject, const PropagationKind &kind,
    const PropagationContext &context, Pool &pool) {
  resetMessages(subject);
  PropagationResult result;
  result.was_completed = true;
  result.propagation_kind_done = kind;
  for (auto &cluster : subject) {
    if (cluster.connectivity.empty()) {
      update_connectivity(cluster);
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
    if (!loopy_propagation(cluster,
                           LoopyPropagationContext{
                               kind, context.max_iterations_loopy_propagation,
                               DEFAULT_VARIATION_TOLLERANCE},
                           pool)) {
      result.was_completed = false;
    }
  }
  return result;
}
} // namespace EFG::strct
