/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/structure/GibbsSampler.h>
#include <EasyFactorGraph/structure/SpecialFactors.h>

#include "Utils.h"

#include <algorithm>
#include <list>
#include <time.h>

namespace EFG::strct {
UniformSampler::UniformSampler() {
  auto random_seed = static_cast<unsigned int>(time(NULL));
  resetSeed(random_seed);
}

std::size_t UniformSampler::sampleFromDiscrete(
    const std::vector<float> &distribution) const {
  float s = this->sample();
  float cumul = 0.f;
  for (std::size_t k = 0; k < distribution.size(); ++k) {
    cumul += distribution[k];
    if (s <= cumul) {
      return k;
    }
  }
  return distribution.size() - 1;
}

void UniformSampler::resetSeed(const std::size_t &newSeed) {
  this->generator.seed(static_cast<unsigned int>(newSeed));
}

namespace {
struct SamplerNode {
  std::size_t *value_in_combination;
  const distribution::UnaryFactor *static_dependencies;

  struct DynamicDependency {
    categoric::VariablePtr sender;
    const std::size_t *sender_value_in_combination;
    distribution::DistributionCnstPtr factor;
  };
  std::vector<DynamicDependency> dynamic_dependencies;
};

std::vector<SamplerNode>
make_sampler_nodes(std::vector<std::size_t> &combination_buffer,
                   const GraphState &state,
                   const categoric::VariablesSoup &vars_order) {
  const std::size_t size = vars_order.size();
  combination_buffer.resize(size);
  SmartMap<categoric::Variable, std::size_t *> combination_values_map;
  for (std::size_t k = 0; k < size; ++k) {
    combination_values_map.emplace(vars_order[k],
                                   &combination_buffer.data()[k]);
  }
  std::vector<SamplerNode> result;
  result.reserve(size);
  for (const auto &[var, val] : state.evidences) {
    *combination_values_map[var] = val;
  }
  for (auto &cluster : state.clusters) {
    for (auto *node : cluster.nodes) {
      update_merged_unaries(*node);
      auto &added_node = result.emplace_back();
      added_node.value_in_combination = combination_values_map[node->variable];
      *added_node.value_in_combination = 0;
      added_node.static_dependencies = node->merged_unaries.get();
      for (const auto &[connected_node, connection] :
           node->active_connections) {
        auto &added_dep = added_node.dynamic_dependencies.emplace_back();
        added_dep.sender = connected_node->variable;
        added_dep.sender_value_in_combination =
            combination_values_map[connected_node->variable];
        added_dep.factor = connection->factor;
      }
    }
    return result;
  }
}

bool have_no_changing_deps(const SamplerNode &subject,
                           const std::set<const std::size_t *> &will_change) {
  for (const auto &dep : subject.dynamic_dependencies) {
    if (exists(will_change, dep.sender_value_in_combination)) {
      return false;
    }
  }
  return true;
}

Task make_sampling_task(const SamplerNode &subject,
                        std::vector<UniformSampler> &engines) {
  return [&subject, &engines](const std::size_t thread_id) {
    std::vector<const distribution::Distribution *> factors = {
        subject.static_dependencies};
    std::list<distribution::Evidence> marginalized;
    for (const auto &dep : subject.dynamic_dependencies) {
      factors.push_back(&marginalized.emplace_back(
          *dep.factor, dep.sender, *dep.sender_value_in_combination));
    }
    distribution::UnaryFactor merged(factors);
    *subject.value_in_combination =
        engines[thread_id].sampleFromDiscrete(merged.getProbabilities());
  };
}

std::vector<Tasks> make_sampling_tasks(const std::vector<SamplerNode> &nodes,
                                       std::vector<UniformSampler> &engines,
                                       const std::optional<std::size_t> &seed,
                                       Pool &pool) {
  engines.resize(pool.size());
  if (std::nullopt != seed) {
    std::size_t s = *seed;
    for (auto &engine : engines) {
      engine.resetSeed(s);
      s += 5;
    }
  }

  std::vector<Tasks> result;
  if (1 == pool.size()) {
    auto &new_tasks = result.emplace_back();
    for (const auto &node : nodes) {
      new_tasks.emplace_back(make_sampling_task(node, engines));
    }
  } else {
    std::list<const SamplerNode *> open;
    for (auto &node : nodes) {
      open.push_back(&node);
    }
    while (!open.empty()) {
      std::set<const std::size_t *> should_not_change;
      std::set<const std::size_t *> will_change;
      auto &new_tasks = result.emplace_back();
      auto open_it = open.begin();
      while (open_it != open.end()) {
        if (!exists(should_not_change, (*open_it)->value_in_combination) &&
            have_no_changing_deps(**open_it, will_change)) {
          will_change.emplace((*open_it)->value_in_combination);
          for (const auto &dep : (*open_it)->dynamic_dependencies) {
            should_not_change.emplace(dep.sender_value_in_combination);
          }
          new_tasks.emplace_back(make_sampling_task(**open_it, engines));
          open_it = open.erase(open_it);
        } else {
          ++open_it;
        }
      }
    }
  }
  return result;
}
} // namespace

std::vector<categoric::Combination>
GibbsSampler::makeSamples(const SamplesGenerationContext &context,
                          const std::size_t threads) {
  std::size_t delta_iterations;
  if (context.delta_iterations) {
    delta_iterations = *context.delta_iterations;
  } else {
    delta_iterations =
        static_cast<std::size_t>(ceil(context.samples_number * 0.1));
  }
  delta_iterations = std::max<std::size_t>(1, delta_iterations);

  ScopedPoolActivator activator(*this, threads);

  std::vector<std::size_t> combination;
  auto sampling_nodes =
      make_sampler_nodes(combination, getState(), getAllVariables());

  std::vector<UniformSampler> engines;
  auto &pool = getPool();
  auto sampling_tasks =
      make_sampling_tasks(sampling_nodes, engines, context.seed, pool);

  std::vector<categoric::Combination> result;
  result.reserve(context.samples_number);
  while (result.size() != context.samples_number) {
    for (std::size_t iter = 0; iter < delta_iterations; ++iter) {
      for (const auto &tasks : sampling_tasks) {
        pool.parallelFor(tasks);
      }
    }
    std::vector<std::size_t> combination_copy = combination;
    result.emplace_back(std::move(combination_copy));
  }
  return result;
}
} // namespace EFG::strct
