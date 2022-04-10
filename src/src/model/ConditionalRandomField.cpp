/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/model/ConditionalRandomField.h>
#include <EasyFactorGraph/trainable/tuners/TunerVisitor.h>

#include "HiddenObservedTuner.h"

namespace EFG::model {
ConditionalRandomField::ConditionalRandomField(
    const ConditionalRandomField &o) {
  absorb(SourceStructure{static_cast<const strct::FactorsAware *>(&o),
                         static_cast<const train::FactorsTunableAware *>(&o)},
         true);
}

ConditionalRandomField::ConditionalRandomField(const RandomField &source,
                                               const bool copy) {
  absorb(
      SourceStructure{static_cast<const strct::FactorsAware *>(&source),
                      static_cast<const train::FactorsTunableAware *>(&source)},
      copy);
}

void ConditionalRandomField::absorb(const SourceStructure &source,
                                    const bool copy) {
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
    train::visit_tuner(
        tuner,
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
  const auto &vars = subject.getFactor().getVariables().getVariables();
  switch (vars.size()) {
  case 1: {
    if (evidences.find(vars.front()) != evidences.end()) {
      throw Error{"Found unary factor attached to permanent evidence: ",
                  vars.front()->name()};
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
      throw Error{"Found factor connecting the permanent evidences: ",
                  first_as_evidence->first->name(), " and ",
                  second_as_evidence->first->name()};
      return;
    }
    train::TunerPtr replacing_tuner;
    if (first_as_evidence == evidences.end()) {
      strct::Node &hidden = *getState_().nodes.find(vars.front())->second;
      replacing_tuner = std::make_unique<train::HiddenObservedTuner>(
          hidden, second_as_evidence, subject.getFactorPtr(),
          getAllVariables());
    } else {
      strct::Node &hidden = *getState_().nodes.find(vars.back())->second;
      replacing_tuner = std::make_unique<train::HiddenObservedTuner>(
          hidden, first_as_evidence, subject.getFactorPtr(), getAllVariables());
    }
    container = std::move(replacing_tuner);
  } break;
  }
}

void ConditionalRandomField::setEvidences(
    const std::vector<std::size_t> &values) {
  const auto &state = getState();
  if (values.size() != state.evidences.size()) {
    throw Error{"Expected ", std::to_string(state.evidences.size()),
                " evidences, but got instead ", values.size()};
  }
  std::size_t k = 0;
  for (const auto &[var, val] : state.evidences) {
    setEvidence(var, values[k]);
    ++k;
  }
}

namespace {
std::vector<std::size_t>
extract_evidences(const categoric::Combination &comb,
                  const std::vector<std::size_t> &evidence_vars_positions) {
  std::vector<std::size_t> result;
  result.reserve(evidence_vars_positions.size());
  const auto &data = comb.data();
  for (const auto pos : evidence_vars_positions) {
    result.push_back(data[pos]);
  }
  return result;
}
} // namespace

std::vector<float> ConditionalRandomField::getWeightsGradient_(
    const train::TrainSet::Iterator &train_set_combinations) {
  // compute alfa part
  std::vector<float> alfas;
  {
    alfas.reserve(tuners.size());
    strct::Tasks tasks;
    for (auto &tuner : tuners) {
      auto &receiver = alfas.emplace_back();
      tasks.emplace_back([&receiver = receiver, &tuner = tuner,
                          &train_set_combinations](const std::size_t) {
        tuner->setTrainSetIterator(train_set_combinations);
        receiver = tuner->getGradientAlpha();
      });
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
    train_set_combinations.forEachSample(
        [this, &betas, &coeff](const categoric::Combination &combination) {
          this->setEvidences(
              extract_evidences(combination, this->evidence_vars_positions));
          propagateBelief(strct::SUM);
          strct::Tasks tasks;
          for (std::size_t t = 0; t < this->tuners.size(); ++t) {
            tasks.emplace_back([&receiver = betas[t], &tuner = tuners[t],
                                &coeff](const std::size_t) {
              receiver = coeff * tuner->getGradientBeta();
            });
          }
          this->getPool().parallelFor(tasks);
        });
  }
  for (std::size_t k = 0; k < alfas.size(); ++k) {
    alfas[k] -= betas[k];
  }
  return alfas;
}
} // namespace EFG::model
