/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/structure/BaselineLoopyPropagator.h>

#include "Utils.h"

#include <algorithm>
#include <list>

namespace EFG::strct {
namespace {
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
  if (variations.size() > 1) {
    auto open = pack_all_tasks(*cluster.connectivity.get());
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
    new_tasks.reserve(cluster.connectivity.get()->size());
    for (auto &task : *cluster.connectivity.get()) {
      new_tasks.emplace_back(make_task(task));
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
  for (auto *node : cluster.nodes) {
    for (auto &[sender, connection] : node->active_connections) {
      connection->message = make_unary(node->variable);
    }
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

bool BaselineLoopyPropagator::propagateBelief(HiddenCluster &subject,
                                              const PropagationKind &kind,
                                              const PropagationContext &context,
                                              Pool &pool) {
  return loopy_propagation(
      subject,
      LoopyPropagationContext{kind, context.max_iterations_loopy_propagation,
                              DEFAULT_VARIATION_TOLLERANCE},
      pool);
}
} // namespace EFG::strct
