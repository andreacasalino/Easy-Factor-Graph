/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/distribution/DistributionConcrete.h>

namespace EFG::distribution {
DistributionConcrete::DistributionConcrete(const EvaluatorPtr &evaluator,
                                           const categoric::Group &vars)
    : variables(vars), evaluator(evaluator) {
  if (nullptr == this->evaluator) {
    throw Error{"Empty evaluator"};
  }
  combinations_map = std::make_shared<CombinationRawValuesMap>();
}

DistributionConcrete::DistributionConcrete(
    const EvaluatorPtr &evaluator, const categoric::Group &vars,
    const CombinationRawValuesMapPtr &map)
    : DistributionConcrete(evaluator, vars) {
  if (nullptr == map) {
    throw Error{"Empty combinations map"};
  }
  combinations_map = map;
}
} // namespace EFG::distribution
