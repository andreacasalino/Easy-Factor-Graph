/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/model/ConditionalRandomField.h>

namespace EFG::model {
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
