/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/trainable/FactorsTunableManager.h>
#include <EasyFactorGraph/trainable/tuners/BinaryTuner.h>
#include <EasyFactorGraph/trainable/tuners/CompositeTuner.h>
#include <EasyFactorGraph/trainable/tuners/UnaryTuner.h>

#include "../structure/Utils.h"

#include <algorithm>

namespace EFG::train {
namespace {
void use_tuner(
    const TunerPtr &subject,
    const std::function<void(const BaseTuner &)> &base_case =
        [](const BaseTuner &tuner) {},
    const std::function<void(const CompositeTuner &)> &composite_case =
        [](const CompositeTuner &tuner) {}) {
  const BaseTuner *as_base = dynamic_cast<const BaseTuner *>(subject.get());
  if (nullptr != as_base) {
    base_case(*as_base);
    return;
  }
  const CompositeTuner *as_composite =
      dynamic_cast<const CompositeTuner *>(subject.get());
  if (nullptr != as_composite) {
    composite_case(*as_composite);
    return;
  }
  throw Error{"Unrecognized Tuner"};
}
} // namespace

std::vector<std::vector<FactorExponentialPtr>>
FactorsTunableAware::getTunableClusters() const {
  std::vector<std::vector<FactorExponentialPtr>> result;
  result.reserve(tuners.size());
  for (const auto &tuner : tuners) {
    auto &cluster = result.emplace_back();
    use_tuner(
        tuner,
        [&cluster](const BaseTuner &tuner) {
          cluster.push_back(tuner.getFactorPtr());
        },
        [&cluster](const CompositeTuner &composite) {
          for (const auto &element : composite.getElements()) {
            cluster.push_back(
                static_cast<const BaseTuner *>(element.get())->getFactorPtr());
          }
        });
  }
  return result;
}

std::vector<float> FactorsTunableAware::getWeights() const {
  std::vector<float> result;
  result.reserve(tuners.size());
  for (const auto &tuner : tuners) {
    result.push_back(tuner->getWeight());
  }
  return result;
}

void FactorsTunableAware::setWeights(const std::vector<float> &weights) {
  if (weights.size() != tuners.size()) {
    throw Error{"Invalid weights"};
  }
  std::size_t pos = 0;
  for (auto &tuner : tuners) {
    tuner->setWeight(weights[pos]);
    ++pos;
  }
}

std::vector<float> FactorsTunableAware::getWeightsGradient(
    const TrainSet::Iterator &train_set_combinations,
    const std::size_t threads) {
  ScopedPoolActivator activator(*this, threads);
  return getWeightsGradient_(train_set_combinations);
}

void FactorsTunableAdder::addTuner(
    train::TunerPtr tuner,
    const std::optional<categoric::VariablesSet> &group_sharing_weight) {
  if (std::nullopt == group_sharing_weight) {
    tuners.emplace_back(std::move(tuner));
    return;
  }
  auto tuners_it = std::find_if(
      tuners.begin(), tuners.end(),
      [&group = *group_sharing_weight](const TunerPtr &tuner) {
        bool is_here = false;
        use_tuner(
            tuner,
            [&is_here, &group](const BaseTuner &tuner) {
              is_here =
                  tuner.getFactor().getVariables().getVariablesSet() == group;
            },
            [&is_here, &group](const CompositeTuner &composite) {
              for (const auto &element : composite.getElements()) {
                const auto *as_base_tuner =
                    static_cast<const BaseTuner *>(element.get());
                if (as_base_tuner->getFactor()
                        .getVariables()
                        .getVariablesSet() == group) {
                  is_here = true;
                  return;
                }
              }
            });
        return is_here;
      });
  CompositeTuner *as_composite =
      dynamic_cast<CompositeTuner *>(tuners_it->get());
  if (nullptr != as_composite) {
    as_composite->addElement(std::move(tuner));
    return;
  }
  TunerPtr new_composite =
      std::make_unique<CompositeTuner>(std::move(*tuners_it), std::move(tuner));
  *tuners_it = std::move(new_composite);
}

namespace {
strct::Node *extract_node(const strct::NodeLocation &location) {
  strct::Node *result;
  strct::visit_location(
      location,
      [&result](const strct::HiddenNodeLocation &location) {
        result = location.node;
      },
      [&result](const strct::EvidenceNodeLocation &location) {
        result = location.node;
      });
  return result;
}
} // namespace

TunerPtr FactorsTunableAdder::makeTuner(const FactorExponentialPtr &factor) {
  auto vars = getAllVariables();
  const auto &factor_vars = factor->getVariables().getVariables();
  switch (factor->getVariables().getVariables().size()) {
  case 1: {
    auto *node = extract_node(*locate(factor_vars.front()));
    return std::make_unique<UnaryTuner>(*node, factor, vars);
  }
  case 2: {
    auto *nodeA = extract_node(*locate(factor_vars.front()));
    auto *nodeB = extract_node(*locate(factor_vars.back()));
    return std::make_unique<BinaryTuner>(*nodeA, *nodeB, factor, vars);
  }
  }
  throw Error{"Invalid tunable factor"};
}

void FactorsTunableAdder::addTunableFactor(
    const FactorExponentialPtr &factor,
    const std::optional<categoric::VariablesSet> &group_sharing_weight) {
  addDistribution(factor);
  auto tuner = makeTuner(factor);
  addTuner(std::move(tuner), group_sharing_weight);
  tunable_factors.emplace(factor);
}

void FactorsTunableAdder::copyTunableFactor(
    const distribution::FactorExponential &factor,
    const std::optional<categoric::VariablesSet> &group_sharing_weight) {
  auto cloned = std::make_shared<distribution::FactorExponential>(factor);
  addTunableFactor(cloned, group_sharing_weight);
}

void set_ones(FactorsTunableAware &subject) {
  std::vector<float> weights = subject.getWeights();
  for (auto &w : weights) {
    w = 1.f;
  }
  subject.setWeights(weights);
}
} // namespace EFG::train
