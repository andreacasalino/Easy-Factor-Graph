/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/distribution/Factor.h>

namespace EFG::distribution {
class BasicEvaluator : public Evaluator {
public:
  BasicEvaluator() = default;

  float evaluate(const float &input) const final { return input; }
};

Factor::Factor(const categoric::Group &vars)
    : DistributionConcrete(std::make_shared<BasicEvaluator>(), vars) {}
} // namespace EFG::distribution
