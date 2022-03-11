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

class CombinationFinder;

class DistributionConcrete : virtual public Distribution {
public:
  /**
   * @param the variables referring to the combinations to search. This
   kind of set should contain the subset of variables
   * describing the domain of distribution
   * @throw if some of the variables describing the distribution domain
   are not contained in containingGroup
   */
  CombinationFinder
  makeFinder(const categoric::VariablesSoup &bigger_group) const;

  /**
   * @brief searches for the image associated to an element in the domain
   * @return the value of the image.
   */
  float evaluate(const categoric::Combination &comb) const;

  /**
   * @return the probabilities associated to each combination in the domain,
   * when assuming only the existance of this distribution. Such probabilities
   * are the normalized images. The order of returned values, refer to the
   * combination order obtained by iterating with the categoric::Range object.
   */
  std::vector<float> getProbabilities() const;

  const Evaluator &getEvaluator() const final { return *evaluator; }
  const categoric::Group &getVariables() const final { return variables; }
  const CombinationRawValuesMap &getCombinationsMap() const final {
    return *combinations_map;
  }

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
