/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/distribution/Distribution.h>

namespace EFG::distribution {
using CombinationRawValuesMapPtr = std::shared_ptr<CombinationRawValuesMap>;

class DistributionConcrete : virtual public Distribution {
public:
  // get finder
  // TODO

  const Evaluator &getEvaluator() const final { return *evaluator; };
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

  CombinationRawValuesMap &getCombinationsMap() final {
    return *combinations_map;
  }

private:
  categoric::Group variables;
  EvaluatorPtr evaluator;
  CombinationRawValuesMapPtr combinations_map;
};
} // namespace EFG::distribution
