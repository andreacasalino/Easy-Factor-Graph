/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/structure/GibbsSampler.h>

#include "Utils.h"

#include <algorithm>
#include <list>
#include <random>

namespace EFG::strct {
namespace {
class UniformSampler {
public:
  UniformSampler() = default;

  std::size_t sampleFromDiscrete(const std::vector<float> &distribution) const {
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

  void resetSeed(const std::size_t &newSeed) {
    this->generator.seed(static_cast<unsigned int>(newSeed));
  }

private:
  inline float sample() const { return this->distribution(this->generator); };

  mutable std::default_random_engine generator;
  mutable std::uniform_real_distribution<float> distribution;
};

struct SamplerNode;
struct SamplerConnection {
  const SamplerNode *sender;
  distribution::DistributionCnstPtr factor;
};
struct SamplerNode {
  Node *node;
  std::size_t *sample_value;
  std::vector<SamplerConnection> connections;
  std::vector<const distribution::Distribution *> static_unaries;
};
std::vector<SamplerNode> make_nodes(const HiddenClusters &clusters,
                                    std::vector<std::size_t> &recipient) {
  std::size_t nodes_size = 0;
  for (const auto &cluster : clusters) {
    nodes_size += cluster.nodes.size();
  }
  recipient.reserve(nodes_size);
  for (std::size_t k = 0; k < nodes_size; ++k) {
    recipient.push_back(0);
  }
  std::vector<SamplerNode> result;
  result.reserve(nodes_size);
  std::size_t k = 0;
  for (const auto &cluster : clusters) {
    for (auto *node : cluster.nodes) {
      auto &added = result.emplace_back();
      added.node = node;
      added.sample_value = &recipient[k];
      added.static_unaries = gather_unaries(*node);
      ++k;
    }
  }
  // compute connections
  for (auto &sampler_node : result) {
    for (const auto &[connected_node, connection] :
         sampler_node.node->active_connections) {
      auto &added = sampler_node.connections.emplace_back();
      added.factor = connection.factor;
      auto result_it = std::find_if(result.begin(), result.end(),
                                    [&connected_node](const SamplerNode &node) {
                                      return node.node == connected_node;
                                    });
      added.sender = &(*result_it);
    }
  }
  return result;
}

std::vector<float> ones(const std::size_t size) {
  std::vector<float> result;
  result.resize(size);
  for (auto &val : result) {
    val = 1.f;
  }
  return result;
}

Task to_task(const SamplerNode &subject,
             const std::vector<UniformSampler> &engines) {
  throw 0; // gestire questione seeds
  Task result = [subject = subject, &engines](const std::size_t th_id) {
    const auto &engine = engines[th_id];
    auto unaries = subject.static_unaries;
    std::vector<distribution::Evidence> evidences;
    evidences.reserve(subject.connections.size());
    for (const auto &connection : subject.connections) {
      auto &added = evidences.emplace_back(*connection.factor,
                                           connection.sender->node->variable,
                                           *connection.sender->sample_value);
      unaries.push_back(&added);
    }
    if (unaries.empty()) {
      *subject.sample_value =
          engine.sampleFromDiscrete(ones(subject.node->variable->size()));
    } else {
      distribution::UnaryFactor merged_unaries(unaries);
      *subject.sample_value =
          engine.sampleFromDiscrete(merged_unaries.getProbabilities());
    }
  };
  return result;
}

Tasks to_tasks(const std::vector<SamplerNode> &subject,
               const std::vector<UniformSampler> &engines) {
  Tasks result;
  result.reserve(subject.size());
  for (const auto &node : subject) {
    result.push_back(to_task(node, engines));
  }
  return result;
}

std::vector<Tasks> make_tasks(const std::vector<SamplerNode> &nodes,
                              std::vector<UniformSampler> &engines,
                              Pool &pool) {
  engines.resize(pool.size());
  std::list<const SamplerNode *> open;
  for (auto &node : nodes) {
    open.push_back(&node);
  }
  std::vector<Tasks> result;
  while (!open.empty()) {
    auto &tasks = result.emplace_back();
    std::set<const SamplerNode *> locked;
    auto it = open.begin();
    while (it != open.end()) {
      if (std::find_if((*it)->connections.begin(), (*it)->connections.end(),
                       [&locked](const SamplerConnection &connection) {
                         return locked.find(connection.sender) != locked.end();
                       }) == (*it)->connections.end()) {
        tasks.push_back(to_task(**it, engines));
        locked.emplace(*it);
        ++it;
      } else {
        it = open.erase(it);
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
  auto sampling_nodes = make_nodes(getState().clusters, combination);
  std::vector<UniformSampler> engines;
  auto &pool = getPool();
  auto sampling_tasks = make_tasks(sampling_nodes, engines, pool);

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
