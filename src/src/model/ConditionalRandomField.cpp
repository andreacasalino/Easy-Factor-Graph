/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/model/ConditionalRandomField.h>
#include <EasyFactorGraph/trainable/tuners/TunerVisitor.h>

#include "HiddenObservedTuner.h"

#include <algorithm>
#include <math.h>

namespace EFG::model {
namespace {
std::vector<std::size_t>
find_positions(const categoric::VariablesSoup &all_vars,
               const categoric::VariablesSet &to_find) {
  std::vector<std::size_t> result;
  result.reserve(to_find.size());
  for (const auto &var : to_find) {
    auto it = std::find(all_vars.begin(), all_vars.end(), var);
    result.push_back(std::distance(all_vars.begin(), it));
  }
  return result;
}
} // namespace

ConditionalRandomField::ConditionalRandomField(const ConditionalRandomField &o)
    : evidence_vars_positions(
          find_positions(o.getAllVariables(), o.getObservedVariables())) {
  absorb(SourceStructure{static_cast<const strct::FactorsConstGetter *>(&o),
                         static_cast<const train::FactorsTunableGetter *>(&o)},
         false);
}

ConditionalRandomField::ConditionalRandomField(const RandomField &source,
                                               bool copy)
    : evidence_vars_positions(find_positions(source.getAllVariables(),
                                             source.getObservedVariables())) {
  absorb(
      SourceStructure{
          static_cast<const strct::FactorsConstGetter *>(&source),
          static_cast<const train::FactorsTunableGetter *>(&source)},
      copy);
}

void ConditionalRandomField::absorb(const SourceStructure &source, bool copy) {
  const auto &evidences = source.factors_structure->getEvidences();
  if (evidences.empty()) {
    throw Error{"ConditionalRandomField must have at least 1 evidence"};
  }
  const auto &const_factors = source.factors_structure->getConstFactors();
  absorbConstFactors(const_factors.begin(), const_factors.end(), copy);
  absorbTunableClusters(*source.factors_tunable_structure, copy);
  for (const auto &[var, val] : evidences) {
    setEvidence(findVariable(var->name()), val);
  }
  // replace tuners of factors connected to an evidence
  for (auto &tuner : tuners) {
    train::visitTuner(
        tuner.get(),
        [this, &tuner](train::BaseTuner &subject) {
          this->replaceIfNeeded(tuner, subject);
        },
        [this](train::CompositeTuner &subject) {
          for (auto &element : subject.getElements()) {
            this->replaceIfNeeded(
                element,
                *dynamic_cast<const train::BaseTuner *>(element.get()));
          }
        });
  }
}

void ConditionalRandomField::replaceIfNeeded(train::TunerPtr &container,
                                             const train::BaseTuner &subject) {
  const auto &evidences = this->getEvidences();
  const auto &vars = subject.getFactor().function().vars().getVariables();
  switch (vars.size()) {
  case 1: {
    if (evidences.find(vars.front()) != evidences.end()) {
      throw Error::make("Found unary factor attached to permanent evidence: ",
                        vars.front()->name());
    }
  } break;
  case 2: {
    auto first_as_evidence = evidences.find(vars.front());
    auto second_as_evidence = evidences.find(vars.back());
    if ((first_as_evidence == evidences.end()) &&
        (second_as_evidence == evidences.end())) {
      return;
    }
    if ((first_as_evidence != evidences.end()) &&
        (second_as_evidence != evidences.end())) {
      throw Error::make("Found factor connecting the permanent evidences: ",
                        first_as_evidence->first->name(), " and ",
                        second_as_evidence->first->name());
      return;
    }
    train::TunerPtr replacing_tuner;
    if (first_as_evidence == evidences.end()) {
      strct::Node &hidden = *stateMutable().nodes.find(vars.front())->second;
      replacing_tuner = std::make_unique<train::HiddenObservedTuner>(
          hidden, second_as_evidence, subject.getFactorPtr(),
          getAllVariables());
    } else {
      strct::Node &hidden = *stateMutable().nodes.find(vars.back())->second;
      replacing_tuner = std::make_unique<train::HiddenObservedTuner>(
          hidden, first_as_evidence, subject.getFactorPtr(), getAllVariables());
    }
    container = std::move(replacing_tuner);
  } break;
  }
}

void ConditionalRandomField::setEvidences(
    const std::vector<std::size_t> &values) {
  const auto &state = this->state();
  if (values.size() != state.evidences.size()) {
    throw Error::make("Expected ", std::to_string(state.evidences.size()),
                      " evidences, but got instead ", values.size());
  }
  std::size_t k = 0;
  for (const auto &[var, val] : state.evidences) {
    setEvidence(var, values[k]);
    ++k;
  }
}

std::vector<float> ConditionalRandomField::getWeightsGradient_(
    const train::TrainSet::Iterator &train_set_combinations) {
  // compute alfa part
  std::vector<float> alfas;
  {
    alfas.resize(tuners.size());
    strct::Tasks tasks;
    std::size_t alfas_pos = 0;
    for (auto &tuner : tuners) {
      tasks.emplace_back([&receiver = alfas[alfas_pos], &tuner = tuner,
                          &train_set_combinations](const std::size_t) {
        receiver = tuner->getGradientAlpha(train_set_combinations);
      });
      ++alfas_pos;
    }
    getPool().parallelFor(tasks);
  }
  // compute beta part
  std::vector<float> betas;
  {
    betas.reserve(tuners.size());
    for (std::size_t k = 0; k < tuners.size(); ++k) {
      betas.push_back(0);
    }
    float coeff = 1.f / static_cast<float>(train_set_combinations.size());
    strct::Tasks tasks;
    for (std::size_t t = 0; t < this->tuners.size(); ++t) {
      tasks.emplace_back([&receiver = betas[t], &tuner = tuners[t],
                          &coeff](const std::size_t) {
        receiver += coeff * tuner->getGradientBeta();
      });
    }
    train_set_combinations.forEachSample(
        [this, &tasks](const auto &combination) {
          {
            std::size_t var_pos = 0;
            for (auto &[var, val] : this->state().evidences) {
              this->setEvidence(
                  var, combination[this->evidence_vars_positions[var_pos]]);
              ++var_pos;
            }
          }
          propagateBelief(strct::PropagationKind::SUM);
          this->getPool().parallelFor(tasks);
        });
  }
  for (std::size_t k = 0; k < alfas.size(); ++k) {
    alfas[k] -= betas[k];
  }
  return alfas;
}

std::vector<std::vector<std::size_t>> ConditionalRandomField::makeTrainSet(
    const GibbsSampler::SamplesGenerationContext &context,
    float range_percentage, const std::size_t threads) {
  if ((range_percentage > 1.f) || (range_percentage < 0)) {
    throw Error{"Invalid range percentage"};
  }

  std::vector<std::size_t> hidden_vars_position =
      find_positions(getAllVariables(), getHiddenVariables());

  std::vector<std::vector<std::size_t>> result;
  auto emplace_samples = [&](const auto &ev) {
    this->setEvidences(ev);
    for (const auto &sample : this->makeSamples(context, threads)) {
      result.push_back(sample);
    }
  };

  const auto evidence_set = getObservedVariables();
  auto evidences_group = categoric::Group{
      categoric::VariablesSoup{evidence_set.begin(), evidence_set.end()}};
  categoric::GroupRange evidences_range(evidences_group);
  if (1.f == range_percentage) {
    categoric::for_each_combination(evidences_range, emplace_samples);
  } else {
    const std::size_t result_max_size = evidences_group.size();
    const std::size_t result_size_approx =
        static_cast<std::size_t>(floorf(result_max_size * range_percentage));
    const std::size_t delta = result_max_size / result_size_approx;
    std::size_t k = 0;
    while (k < result_max_size) {
      emplace_samples(*evidences_range);
      if ((k + delta) >= result_max_size) {
        break;
      }
      for (std::size_t i = 0; i < delta; ++i) {
        ++evidences_range;
      }
      k += delta;
    }
  }
  return result;
}
} // namespace EFG::model
