/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/trainable/tuners/BaseTuner.h>

namespace EFG::train {
BaseTuner::BaseTuner(const std::shared_ptr<factor::FactorExponential> &factor,
                     const categoric::VariablesSoup &variables_in_model)
    : factor(factor), finder(factor->makeFinder(variables_in_model)) {}

float BaseTuner::getGradientAlpha(const TrainSet::Iterator &iter) {
  if (!alpha_part.has_value() || (alpha_part->train_set_iterator != &iter)) {
    auto &[iter_ptr, val] = alpha_part.emplace();
    val = 0;
    const float coeff = 1.f / static_cast<float>(iter.size());
    iter.forEachSample([&finder = this->finder, &value = val,
                        &coeff](const std::vector<std::size_t> &comb) {
      value += coeff * finder.findImage(comb);
    });
  }
  return alpha_part->value;
}

float BaseTuner::dotProduct(const std::vector<float> &prob) const {
  float dot = 0;
  auto prob_it = prob.begin();
  factor->function().forEachCombination<false>([&](const auto &, float img) {
    dot += *prob_it * img;
    ++prob_it;
  });
  return dot;
}
} // namespace EFG::train
