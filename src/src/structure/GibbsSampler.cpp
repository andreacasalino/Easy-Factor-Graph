/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/structure/GibbsSampler.h>
#include <EasyFactorGraph/structure/SpecialFactors.h>

#include <algorithm>
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

void UniformSampler::resetSeed(std::size_t newSeed) {
  this->generator.seed(static_cast<unsigned int>(newSeed));
}

bool GibbsSampler::SamplerNode::noChangingDeps(
    const std::unordered_set<const std::size_t *> &will_change) const {
  auto it =
      std::find_if(dynamic_dependencies.begin(), dynamic_dependencies.end(),
                   [&will_change](const DynamicDependency &dep) {
                     return will_change.find(dep.sender_value_in_combination) !=
                            will_change.end();
                   });
  return it == dynamic_dependencies.end();
}

std::vector<GibbsSampler::SamplerNode> GibbsSampler::makeSamplerNodes(
    std::vector<std::size_t> &combination_buffer) const {
  const auto vars_order = getAllVariables();
  const std::size_t size = vars_order.size();
  combination_buffer.resize(size);
  SmartMap<categoric::Variable, std::size_t *> combination_values_map;
  for (std::size_t k = 0; k < size; ++k) {
    auto *comb_ptr = &combination_buffer[k];
    *comb_ptr = 0;
    combination_values_map.emplace(vars_order[k], comb_ptr);
  }
  std::vector<SamplerNode> result;
  result.reserve(size);
  const auto &state = this->state();
  for (const auto &[var, val] : state.evidences) {
    *combination_values_map.find(var)->second = val;
  }
  for (auto &cluster : state.clusters) {
    for (auto *node : cluster.nodes) {
      node->updateMergedUnaries();
      auto &added_node = result.emplace_back();
      added_node.value_in_combination =
          combination_values_map.find(node->variable)->second;
      added_node.static_dependencies = node->merged_unaries.get();
      for (const auto &[connected_node, connection] :
           node->active_connections) {
        auto &added_dep = added_node.dynamic_dependencies.emplace_back();
        added_dep.sender = connected_node->variable;
        added_dep.sender_value_in_combination =
            combination_values_map.find(connected_node->variable)->second;
        added_dep.factor = connection.factor;
      }
    }
  }
  return result;
}

namespace {
Task make_sampling_task(const GibbsSampler::SamplerNode &subject,
                        std::vector<UniformSampler> &engines) {
  return [&subject, &engines](const std::size_t thread_id) {
    std::vector<const factor::Immutable *> factors = {
        subject.static_dependencies};
    std::list<factor::Evidence> marginalized;
    for (const auto &dep : subject.dynamic_dependencies) {
      factors.push_back(&marginalized.emplace_back(
          *dep.factor, dep.sender, *dep.sender_value_in_combination));
    }
    factor::MergedUnaries merged(factors);
    *subject.value_in_combination =
        engines[thread_id].sampleFromDiscrete(merged.getProbabilities());
  };
}

std::vector<Tasks>
make_sampling_tasks(const std::vector<GibbsSampler::SamplerNode> &nodes,
                    std::vector<UniformSampler> &engines,
                    const std::optional<std::size_t> &seed, Pool &pool) {
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
    std::list<const GibbsSampler::SamplerNode *> open;
    for (auto &node : nodes) {
      open.push_back(&node);
    }
    while (!open.empty()) {
      std::unordered_set<const std::size_t *> should_not_change;
      std::unordered_set<const std::size_t *> will_change;
      auto &new_tasks = result.emplace_back();
      auto open_it = open.begin();
      while (open_it != open.end()) {
        if ((should_not_change.find((*open_it)->value_in_combination) ==
             should_not_change.end()) &&
            (*open_it)->noChangingDeps(will_change)) {
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

std::pair<std::size_t, std::size_t>
delta_and_burn_out(const GibbsSampler::SamplesGenerationContext &context) {
  std::size_t delta_iterations =
      context.delta_iterations.has_value()
          ? context.delta_iterations.value()
          : static_cast<std::size_t>(ceil(context.samples_number * 0.1));
  delta_iterations = std::max<std::size_t>(1, delta_iterations);

  std::size_t burn_out = context.transient.has_value()
                             ? context.transient.value()
                             : 10 * delta_iterations;

  return std::make_pair(delta_iterations, burn_out);
}

void evolve_samples(strct::Pool &pool, const std::vector<Tasks> &sample_jobs,
                    std::size_t iterations) {
  for (std::size_t iter = 0; iter < iterations; ++iter) {
    for (const auto &tasks : sample_jobs) {
      pool.parallelFor(tasks);
    }
  }
}
} // namespace

std::vector<std::vector<std::size_t>>
GibbsSampler::makeSamples(const SamplesGenerationContext &context,
                          const std::size_t threads) {
  ScopedPoolActivator activator(*this, threads);

  auto [delta_iterations, burn_out] = delta_and_burn_out(context);

  std::vector<std::size_t> combination;
  auto sampling_nodes = makeSamplerNodes(combination);

  std::vector<UniformSampler> engines;
  auto &pool = getPool();
  auto sampling_tasks =
      make_sampling_tasks(sampling_nodes, engines, context.seed, pool);

  evolve_samples(pool, sampling_tasks, burn_out);
  std::vector<std::vector<std::size_t>> result;
  result.reserve(context.samples_number);
  while (result.size() != context.samples_number) {
    result.push_back(combination);
    evolve_samples(pool, sampling_tasks, delta_iterations);
  }
  return result;
}
} // namespace EFG::strct
