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
bool have_no_changing_deps(const ConnectionAndDependencies &subject,
                           const std::set<const Connection *> &will_change) {
  for (const auto *dep : subject.dependencies) {
    if (exists(will_change, dep)) {
      return false;
    }
  }
  return true;
}

std::vector<Tasks>
compute_loopy_order(HiddenCluster &cluster, const PropagationKind &kind,
                    std::vector<MessageVariation> &variations) {
  auto make_task = [&kind, &variations](ConnectionAndDependencies &subject) {
    return [&task = subject, &kind, &variations](const std::size_t th_id) {
      auto &variation = variations[th_id];
      auto candidate = *update_message(task, kind);
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
      std::set<const Connection *> should_not_change;
      std::set<const Connection *> will_change;
      auto &new_tasks = result.emplace_back();
      auto open_it = open.begin();
      while (open_it != open.end()) {
        if (!exists(should_not_change, (*open_it)->connection) &&
            have_no_changing_deps(**open_it, will_change)) {
          will_change.emplace((*open_it)->connection);
          for (const auto *dep : (*open_it)->dependencies) {
            should_not_change.emplace(dep);
          }
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
} // namespace

bool BaselineLoopyPropagator::propagateBelief(HiddenCluster &subject,
                                              const PropagationKind &kind,
                                              const PropagationContext &context,
                                              Pool &pool) {
  auto open = pack_all_tasks(*subject.connectivity.get());
  // set message to ones
  for (auto *node : subject.nodes) {
    for (auto &[sender, connection] : node->active_connections) {
      connection->message = make_unary(node->variable);
    }
  }
  std::vector<MessageVariation> variations;
  variations.resize(pool.size());
  auto order = compute_loopy_order(subject, kind, variations);
  for (std::size_t iter = 0; iter < context.max_iterations_loopy_propagation;
       ++iter) {
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
} // namespace EFG::strct
