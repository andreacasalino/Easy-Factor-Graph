/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/distribution/Distribution.h>

namespace EFG::distribution {
float Distribution::evaluate(const categoric::Combination &comb) const {
  const auto &map = getCombinationsMap();
  auto it = map.find(comb);
  const auto &evaluator = getEvaluator();
  if (it == map.end()) {
    return evaluator.evaluate(0);
  }
  return evaluate.evaluate(it->second);
}

std::vector<float> Distribution::getProbabilities() const {

  if (this->values->empty()) {
    return std::vector<float>(this->group->size(),
                              1.f / static_cast<float>(this->group->size()));
  }
  std::vector<float> probs;
  const std::size_t jointSize = this->group->size();
  probs.reserve(jointSize);
  if (jointSize == this->values->size()) {
    for (auto it = this->values->begin(); it != this->values->end(); ++it) {
      probs.push_back(this->evaluator->evaluate(it->second));
    }
  } else {
    categoric::Range jointDomain(this->group->getVariables());
    iterator::forEach(
        jointDomain, [this, &probs](categoric::Range &jointDomain) {
          auto it = this->values->find(jointDomain.get());
          if (it == this->values->end()) {
            probs.push_back(0.f);
          } else {
            probs.push_back(this->evaluator->evaluate(it->second));
          }
        });
  }
  // normalize values
  float sum = 0.f;
  std::for_each(probs.begin(), probs.end(),
                [&sum](const float &v) { sum += v; });
  std::for_each(probs.begin(), probs.end(), [&sum](float &v) { v /= sum; });
  return probs;
}
} // namespace EFG::distribution
