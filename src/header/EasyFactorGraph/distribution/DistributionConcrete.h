/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Distribution.h>

namespace EFG::distribution {
using CombinationRawValuesMapPtr = std::shared_ptr<CombinationRawValuesMap>;

class DistributionConcrete : virtual public Distribution {
public:
  DistributionConcrete(const DistributionConcrete &o) = delete;
  DistributionConcrete &operator==(const DistributionConcrete &) = delete;
  DistributionConcrete(DistributionConcrete &&o) = delete;
  DistributionConcrete &operator==(DistributionConcrete &&) = delete;

  CombinationFinder
  makeFinder(const categoric::VariablesSoup &bigger_group) const final;

  const Evaluator &getEvaluator() const final { return *evaluator; }
  const categoric::Group &getGroup() const final { return variables; }
  const CombinationRawValuesMap &getCombinationsMap() const final {
    return *combinations_map;
  }

  /**
   * @brief Replaces the variables this distribution should refer to.
   */
  void replaceVariables(const categoric::VariablesSoup &new_variables) {
    variables.replaceVariables(new_variables);
  };

protected:
  DistributionConcrete(const EvaluatorPtr &evaluator,
                       const categoric::Group &vars);

  DistributionConcrete(const EvaluatorPtr &evaluator,
                       const categoric::Group &vars,
                       const CombinationRawValuesMapPtr &map);

  CombinationRawValuesMap &getCombinationsMap_() final {
    return *combinations_map;
  }
  Evaluator &getEvaluator_() final { return *evaluator; };

private:
  categoric::Group variables;
  EvaluatorPtr evaluator;
  CombinationRawValuesMapPtr combinations_map;
};
} // namespace EFG::distribution
