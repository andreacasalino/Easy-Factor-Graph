/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/distribution/FactorExponential.h>
#include <math.h>

namespace EFG::distribution {
class ExponentialEvaluator : public Evaluator {
public:
  ExponentialEvaluator(const float w = 1.f) : weigth(w){};

  float evaluate(const float &input) const final {
    return expf(weigth * input);
  }

  void setWeight(float w) { weigth = w; };
  float getWeight() const { return weigth; };

private:
  float weigth;
};

FactorExponential::FactorExponential(const Factor &factor)
    : FactorExponential(factor, 1.f) {}

namespace {
CombinationRawValuesMapPtr make_exponential_distribution(const Factor &factor) {
  const auto &factor_map = factor.getCombinationsMap();
  CombinationRawValuesMapPtr result =
      std::make_shared<CombinationRawValuesMap>();
  if (factor_map.size() == factor.getVariables().size()) {
    *result = factor_map;
  } else {
    categoric::GroupRange range(factor.getVariables());
    for_each_combination(range, [&factor_map, &result = *result](
                                    const categoric::Combination &comb) {
      auto factor_map_it = factor_map.find(comb);
      if (factor_map_it == factor_map.end()) {
        result[comb] = 0;
      } else {
        result[comb] = factor_map_it->second;
      }
    });
  }
  return result;
}
} // namespace

FactorExponential::FactorExponential(const Factor &factor, const float weigth)
    : DistributionConcrete(std::make_shared<ExponentialEvaluator>(weigth),
                           factor.getVariables(),
                           make_exponential_distribution(factor)) {}

void FactorExponential::setWeight(float w) {
  static_cast<ExponentialEvaluator &>(getEvaluator_()).setWeight(w);
}

float FactorExponential::getWeight() const {
  return static_cast<const ExponentialEvaluator &>(getEvaluator()).getWeight();
}
} // namespace EFG::distribution
