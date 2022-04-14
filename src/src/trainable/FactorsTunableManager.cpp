/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/trainable/FactorsTunableManager.h>
#include <EasyFactorGraph/trainable/tuners/TunerVisitor.h>
#include <EasyFactorGraph/trainable/tuners/UnaryTuner.h>

#include "../structure/Utils.h"

#include <algorithm>

namespace EFG::train {
std::vector<std::vector<FactorExponentialPtr>>
FactorsTunableAware::getTunableClusters() const {
  std::vector<std::vector<FactorExponentialPtr>> result;
  result.reserve(tuners.size());
  for (const auto &tuner : tuners) {
    auto &cluster = result.emplace_back();
    visit_tuner(
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

TunerPtr &FactorsTunableAdder::findTuner(
    const categoric::VariablesSet &tuned_vars_group) {
  auto tuners_it = std::find_if(
      tuners.begin(), tuners.end(),
      [&group = tuned_vars_group](const TunerPtr &tuner) {
        bool is_here = false;
        visit_tuner(
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
  if (tuners_it == tuners.end()) {
    throw Error{"Tuner not found"};
  }
  return *tuners_it;
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
  tunable_factors.emplace(factor);
  if (std::nullopt == group_sharing_weight) {
    tuners.emplace_back(std::move(tuner));
    return;
  }
  auto &tuner_sharing_w = findTuner(*group_sharing_weight);
  visit_tuner(
      tuner_sharing_w,
      [&tuner, &tuner_sharing_w](BaseTuner &) {
        std::unique_ptr<CompositeTuner> new_composite =
            std::make_unique<CompositeTuner>(std::move(tuner_sharing_w),
                                             std::move(tuner));
        tuner_sharing_w = std::move(new_composite);
      },
      [&tuner](CompositeTuner &tuner_sharing) {
        tuner_sharing.addElement(std::move(tuner));
      });
}

void FactorsTunableAdder::copyTunableFactor(
    const distribution::FactorExponential &factor,
    const std::optional<categoric::VariablesSet> &group_sharing_weight) {
  auto cloned = std::make_shared<distribution::FactorExponential>(factor);
  addTunableFactor(cloned, group_sharing_weight);
}

void FactorsTunableAdder::absorbTunableClusters(
    const FactorsTunableAware &source, const bool copy) {
  for (const auto &cluster : source.getTunableClusters()) {
    try {
      const auto &front_factor = cluster.front();
      if (copy) {
        copyTunableFactor(*front_factor);
      } else {
        addTunableFactor(front_factor);
      }
      const auto &front_vars = front_factor->getVariables().getVariablesSet();
      for (std::size_t k = 1; k < cluster.size(); ++k) {
        if (copy) {
          copyTunableFactor(*cluster[k], front_vars);
        } else {
          addTunableFactor(cluster[k], front_vars);
        }
      }
    } catch (...) {
    }
  }
}

void set_ones(FactorsTunableAware &subject) {
  std::vector<float> weights = subject.getWeights();
  for (auto &w : weights) {
    w = 1.f;
  }
  subject.setWeights(weights);
}
} // namespace EFG::train
