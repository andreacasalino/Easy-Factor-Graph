/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/trainable/tuners/BaseTuner.h>

namespace EFG::train {
BaseTuner::BaseTuner(
    const std::shared_ptr<distribution::FactorExponential> &factor,
    const categoric::VariablesSoup &variables_in_model)
    : factor(factor), finder(factor->makeFinder(variables_in_model)) {}

void BaseTuner::setTrainSetIterator(const TrainSet::Iterator &iter,
                                    const categoric::VariablesSet &vars) {
  if (&iter == train_set_iterator) {
    return;
  }
  train_set_iterator = &iter;
  gradientAlpha = 0;
  const float coeff =
      1.f / static_cast<float>(this->train_set_iterator->size());
  train_set_iterator->forEachSample(
      [this, &coeff](const categoric::Combination &comb) {
        this->gradientAlpha += coeff * this->finder.find(comb).value;
      });
}

float BaseTuner::dotProduct(const std::vector<float> &prob) const {
  float dot = 0.f;
  auto prob_it = prob.begin();
  auto &evaluator = factor->getEvaluator();
  for (const auto &[comb, val] : factor->getCombinationsMap()) {
    dot += *prob_it * evaluator.evaluate(val);
    ++prob_it;
  }
  return dot;
}
} // namespace EFG::train
