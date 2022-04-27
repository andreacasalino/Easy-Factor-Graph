/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/distribution/Distribution.h>

namespace EFG::distribution {
float Distribution::evaluate(const categoric::Combination &comb) const {
  const auto &map = getCombinationsMap();
  auto it = map.find(comb);
  const auto &evaluator = getEvaluator();
  if (it == map.end()) {
    return evaluator.evaluate(0);
  }
  return evaluator.evaluate(it->second);
}

std::vector<float> Distribution::getProbabilities() const {
  const auto &map = getCombinationsMap();
  const auto &vars = getGroup();
  const auto jointSize = vars.size();
  if (map.empty()) {
    return std::vector<float>(jointSize, 1.f / static_cast<float>(jointSize));
  }
  std::vector<float> probs;
  probs.reserve(jointSize);
  if (jointSize == map.size()) {
    const auto &evaluator = getEvaluator();
    for (const auto &[comb, raw_val] : map) {
      probs.push_back(evaluator.evaluate(raw_val));
    }
  } else {
    categoric::GroupRange jointDomain(vars);
    categoric::for_each_combination(
        jointDomain, [this, &probs](const categoric::Combination &comb) {
          probs.push_back(this->evaluate(comb));
        });
  }
  // normalize values
  float sum = 0.f;
  for (const auto &val : probs) {
    sum += val;
  }
  for (auto &val : probs) {
    val /= sum;
  }
  return probs;
}
} // namespace EFG::distribution
