/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/misc/Visitor.h>
#include <EasyFactorGraph/trainable/FactorsTunableManager.h>
#include <EasyFactorGraph/trainable/tuners/BinaryTuner.h>
#include <EasyFactorGraph/trainable/tuners/CompositeTuner.h>
#include <EasyFactorGraph/trainable/tuners/TunerVisitor.h>
#include <EasyFactorGraph/trainable/tuners/UnaryTuner.h>

#include <algorithm>

namespace EFG::train {
namespace {
FactorExponentialPtr extract_factor(const Tuner &subject) {
  return static_cast<const BaseTuner &>(subject).getFactorPtr();
}
} // namespace

std::vector<std::variant<FactorExponentialPtr, TunableClusters>>
FactorsTunableGetter::getTunableClusters() const {
  std::vector<std::variant<FactorExponentialPtr, TunableClusters>> result;
  for (const auto &tuner : tuners) {
    visitTuner(
        tuner.get(),
        [&result](const BaseTuner &base) {
          result.emplace_back(extract_factor(base));
        },
        [&result](const CompositeTuner &composite) {
          TunableClusters cluster;
          for (const auto &f : composite.getElements()) {
            cluster.push_back(extract_factor(*f));
          }
          result.emplace_back(std::move(cluster));
        });
  }
  return result;
}

std::vector<float> FactorsTunableGetter::getWeights() const {
  std::vector<float> result;
  result.reserve(tuners.size());
  for (const auto &tuner : tuners) {
    result.push_back(tuner->getWeight());
  }
  return result;
}

void FactorsTunableGetter::setWeights(const std::vector<float> &weights) {
  if (weights.size() != tuners.size()) {
    throw Error{"Invalid weights"};
  }
  for (std::size_t k = 0; k < tuners.size(); ++k) {
    tuners[k]->setWeight(weights[k]);
  }
  resetBelief();
}

std::vector<float> FactorsTunableGetter::getWeightsGradient(
    const TrainSet::Iterator &train_set_combinations,
    const std::size_t threads) {
  ScopedPoolActivator activator(*this, threads);
  return getWeightsGradient_(train_set_combinations);
}

Tuners::iterator FactorsTunableInserter::findTuner(
    const categoric::VariablesSet &tuned_vars_group) {
  auto tuners_it = std::find_if(
      tuners.begin(), tuners.end(),
      [&group = tuned_vars_group](const TunerPtr &tuner) {
        bool is_here = false;
        visitTuner(
            tuner.get(),
            [&is_here, &group](const BaseTuner &tuner) {
              is_here = tuner.getFactor().function().vars().getVariablesSet() ==
                        group;
            },
            [&is_here, &group](const CompositeTuner &composite) {
              for (const auto &element : composite.getElements()) {
                if (extract_factor(*element)
                        ->function()
                        .vars()
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
  return tuners_it;
}

TunerPtr FactorsTunableInserter::makeTuner(const FactorExponentialPtr &factor) {
  auto vars = getAllVariables();
  const auto &factor_vars = factor->function().vars().getVariables();
  switch (factor_vars.size()) {
  case 1: {
    auto *node = locate(factor_vars.front())->node;
    return std::make_unique<UnaryTuner>(*node, factor, vars);
  }
  case 2: {
    auto *nodeA = locate(factor_vars.front())->node;
    auto *nodeB = locate(factor_vars.back())->node;
    return std::make_unique<BinaryTuner>(*nodeA, *nodeB, factor, vars);
  }
  }
  throw Error{"Invalid tunable factor"};
}

void FactorsTunableInserter::addTunableFactor(
    const FactorExponentialPtr &factor,
    const std::optional<categoric::VariablesSet> &group_sharing_weight) {
  addDistribution(factor);
  auto tuner = makeTuner(factor);
  tunable_factors.emplace(factor);
  if (std::nullopt == group_sharing_weight) {
    tuners.emplace_back(std::move(tuner));
    return;
  }
  auto other_it = findTuner(*group_sharing_weight);
  visitTuner(
      other_it->get(),
      [&tuner, &other_it](BaseTuner &) {
        std::unique_ptr<CompositeTuner> new_composite =
            std::make_unique<CompositeTuner>(std::move(*other_it),
                                             std::move(tuner));
        *other_it = std::move(new_composite);
        other_it->get()->setWeight(other_it->get()->getWeight());
      },
      [&tuner](CompositeTuner &tuner_sharing) {
        tuner_sharing.addElement(std::move(tuner));
        tuner_sharing.setWeight(tuner_sharing.getWeight());
      });
}

void FactorsTunableInserter::copyTunableFactor(
    const factor::FactorExponential &factor,
    const std::optional<categoric::VariablesSet> &group_sharing_weight) {
  auto cloned = std::make_shared<factor::FactorExponential>(factor);
  addTunableFactor(cloned, group_sharing_weight);
}

void FactorsTunableInserter::absorbTunableClusters(
    const FactorsTunableGetter &source, bool copy) {
  auto insertFactor =
      [copy = copy, this](
          const FactorExponentialPtr &factor,
          const std::optional<categoric::VariablesSet> &group_sharing_weight) {
        if (copy) {
          copyTunableFactor(*factor, group_sharing_weight);
        } else {
          addTunableFactor(factor, group_sharing_weight);
        }
      };

  for (const auto &cluster : source.getTunableClusters()) {
    try {
      VisitorConst<FactorExponentialPtr, TunableClusters>{
          [&insertFactor](const FactorExponentialPtr &base) {
            insertFactor(base, std::nullopt);
          },
          [&insertFactor](const TunableClusters &composite) {
            auto front_factor = composite.front();
            const auto &vars =
                front_factor->function().vars().getVariablesSet();
            insertFactor(front_factor, std::nullopt);
            std::for_each(
                composite.begin() + 1, composite.end(),
                [&insertFactor, &vars](const FactorExponentialPtr &tuner) {
                  insertFactor(tuner, vars);
                });
          }}
          .visit(cluster);
    } catch (...) {
    }
  }
}

void set_ones(FactorsTunableGetter &subject) {
  std::vector<float> weights = subject.getWeights();
  for (auto &w : weights) {
    w = 1.f;
  }
  subject.setWeights(weights);
}
} // namespace EFG::train
