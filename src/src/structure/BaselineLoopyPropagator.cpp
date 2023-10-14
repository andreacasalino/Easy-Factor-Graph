/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/structure/BaselineLoopyPropagator.h>

#include <algorithm>

namespace EFG::strct {
namespace {
bool have_no_changing_deps(
    const HiddenCluster::TopologyInfo &subject,
    const std::unordered_set<const Node::Connection *> &will_change) {
  auto it =
      std::find_if(subject.dependencies.begin(), subject.dependencies.end(),
                   [&will_change](const Node::Connection *dep) {
                     return will_change.find(dep) != will_change.end();
                   });
  return it == subject.dependencies.end();
}

using Infoes = std::vector<std::vector<HiddenCluster::TopologyInfo *>>;

Infoes compute_loopy_order(HiddenCluster &cluster,
                           std::vector<float> &variations) {
  Infoes result;
  if (variations.size() > 1) {
    std::list<HiddenCluster::TopologyInfo *> open;
    for (auto &info : *cluster.connectivity.get()) {
      open.push_back(&info);
    }
    // multithreaded computation
    while (!open.empty()) {
      std::unordered_set<const Node::Connection *> should_not_change;
      std::unordered_set<const Node::Connection *> will_change;
      auto &new_tasks = result.emplace_back();
      auto open_it = open.begin();
      while (open_it != open.end()) {
        if ((should_not_change.find((*open_it)->connection) ==
             should_not_change.end()) &&
            have_no_changing_deps(**open_it, will_change)) {
          will_change.emplace((*open_it)->connection);
          for (const auto *dep : (*open_it)->dependencies) {
            should_not_change.emplace(dep);
          }
          new_tasks.push_back(*open_it);
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
      new_tasks.push_back(&task);
    }
  }
  return result;
}

static constexpr float DEFAULT_VARIATION_TOLLERANCE = static_cast<float>(1e-3);
} // namespace

bool BaselineLoopyPropagator::propagateBelief(HiddenCluster &subject,
                                              PropagationKind kind,
                                              const PropagationContext &context,
                                              Pool &pool) {
  // set message to ones
  for (auto *node : subject.nodes) {
    for (auto &[sender, connection] : node->active_connections) {
      connection.message =
          std::make_unique<factor::MergedUnaries>(node->variable);
    }
  }
  std::vector<float> variations;
  variations.resize(pool.size());
  auto order = compute_loopy_order(subject, variations);
  for (std::size_t iter = 0; iter < context.max_iterations_loopy_propagation;
       ++iter) {
    for (auto &variation : variations) {
      variation = 0;
    }
    for (auto &tasks : order) {
      Tasks lambdas;
      lambdas.reserve(tasks.size());
      for (auto *task : tasks) {
        lambdas.emplace_back(
            [task = task, kind = kind, &variations](const std::size_t th_id) {
              auto &variation = variations[th_id];
              auto candidate = task->updateMessage(kind).value();
              variation = std::max<float>(variation, candidate);
            });
      }
      pool.parallelFor(lambdas);
    }
    if (*std::max_element(variations.begin(), variations.end()) <
        DEFAULT_VARIATION_TOLLERANCE) {
      return true;
    }
  }
  return false;
}
} // namespace EFG::strct
