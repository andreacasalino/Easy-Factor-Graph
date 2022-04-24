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
struct SamplerNode;
struct SamplerConnection {
  const SamplerNode *sender;
  distribution::DistributionCnstPtr factor;
};

distribution::UnaryFactor make_static_unaries(Node &n) {
  auto unaries = gather_unaries(n);
  if (unaries.empty()) {
    return distribution::UnaryFactor{n.variable};
  }
  return distribution::UnaryFactor{unaries};
}

struct SamplerNode {
  SamplerNode(Node &n, std::size_t &c)
      : node(&n), combination_value(&c),
        static_unaries(make_static_unaries(n)) {}

  const Node *node;
  std::size_t *combination_value;
  std::vector<SamplerConnection> connections;
  distribution::UnaryFactor static_unaries;
};
using SamplerNodePtr = std::unique_ptr<SamplerNode>;
using SamplerNodes = std::vector<SamplerNodePtr>;
SamplerNodes make_nodes(const HiddenClusters &clusters,
                        const categoric::VariablesSet &hidden_set_order,
                        std::vector<std::size_t> &combination) {
  std::size_t nodes_size = 0;
  for (const auto &cluster : clusters) {
    nodes_size += cluster.nodes.size();
  }
  combination.resize(nodes_size);
  for (auto &comb : combination) {
    comb = 0;
  }
  SamplerNodes result;
  result.reserve(nodes_size);
  for (const auto &cluster : clusters) {
    for (auto *node : cluster.nodes) {
      std::size_t position = std::distance(
          hidden_set_order.begin(), hidden_set_order.find(node->variable));
      SamplerNodePtr new_node =
          std::make_unique<SamplerNode>(*node, combination[position]);
      result.emplace_back(std::move(new_node));
    }
  }
  // compute connections
  for (auto &sampler_node : result) {
    for (const auto &[connected_node, connection] :
         sampler_node->node->active_connections) {
      auto &added = sampler_node->connections.emplace_back();
      added.factor = connection->factor;
      auto result_it = std::find_if(
          result.begin(), result.end(),
          [&connected_node = connected_node](const SamplerNodePtr &node) {
            return node->node == connected_node;
          });
      added.sender = (*result_it).get();
    }
  }
  return result;
}

Task to_sampling_task(const SamplerNode &subject,
                      const std::vector<UniformSampler> &engines) {
  return [subject = subject, &engines](const std::size_t th_id) {
    const auto &engine = engines[th_id];
    std::vector<const distribution::Distribution *> contributions = {
        &subject.static_unaries};
    std::list<distribution::Evidence> incomings;
    for (const auto &connection : subject.connections) {
      const auto &added = incomings.emplace_back(
          *connection.factor, connection.sender->node->variable,
          *connection.sender->combination_value);
      contributions.push_back(&added);
    }
    distribution::UnaryFactor merged(contributions);
    auto sampled = engine.sampleFromDiscrete(merged.getProbabilities());
    *subject.combination_value = sampled;
  };
}

bool have_no_changing_deps(const SamplerNode &subject,
                           const std::set<const SamplerNode *> &will_change) {
  for (const auto &dep : subject.connections) {
    if (exists(will_change, dep.sender)) {
      return false;
    }
  }
  return true;
}

std::vector<Tasks> make_sampling_tasks(const SamplerNodes &nodes,
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
      new_tasks.emplace_back(to_sampling_task(*node, engines));
    }
  } else {
    std::list<const SamplerNode *> open;
    for (auto &node : nodes) {
      open.push_back(node.get());
    }
    while (!open.empty()) {
      std::set<const SamplerNode *> should_not_change;
      std::set<const SamplerNode *> will_change;
      auto &new_tasks = result.emplace_back();
      auto open_it = open.begin();
      while (open_it != open.end()) {
        if (!exists(should_not_change, *open_it) &&
            have_no_changing_deps(**open_it, will_change)) {
          will_change.emplace(*open_it);
          for (const auto &dep : (*open_it)->connections) {
            should_not_change.emplace(dep.sender);
          }
          new_tasks.emplace_back(to_sampling_task(**open_it, engines));
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
GibbsSampler::getHiddenSetSamples(const SamplesGenerationContext &context,
                                  const std::size_t threads) {
  std::size_t delta_iterations;
  if (context.delta_iterations) {
    delta_iterations = *context.delta_iterations;
  } else {
    delta_iterations =
        static_cast<std::size_t>(ceil(context.samples_number * 0.1));
  }
  if (0 == delta_iterations) {
    delta_iterations = 1;
  }

  ScopedPoolActivator activator(*this, threads);

  std::vector<std::size_t> combination;
  auto sampling_nodes =
      make_nodes(getState().clusters, getHiddenVariables(), combination);
  if (sampling_nodes.empty()) {
    throw Error{"Cannot generate samples of a model having an empty hidden"};
  }

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
