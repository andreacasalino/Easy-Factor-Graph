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

float BaseTuner::getGradientAlpha(const TrainSet::Iterator &iter) {
  if (!alpha_part.has_value() || (alpha_part->train_set_iterator != &iter)) {
    auto &value = alpha_part.emplace();
    value.train_set_iterator = &iter;
    value.value = 0;
    const float coeff = 1.f / static_cast<float>(iter.size());
    iter.forEachSample([&finder = this->finder, &value = value.value,
                        &coeff](const categoric::Combination &comb) {
      value += coeff * finder.find(comb).map_iterator->second;
    });
  }
  return alpha_part->value;
}

float BaseTuner::dotProduct(const std::vector<float> &prob) const {
  float dot = 0.f;
  auto prob_it = prob.begin();
  for (const auto &[comb, val] : factor->getCombinationsMap()) {
    dot += *prob_it * val;
    ++prob_it;
  }
  return dot;
}
} // namespace EFG::train
