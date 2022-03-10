/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/DistributionConcrete.h>

namespace EFG::distribution {
/**
 * @brief An object used to search for big combinations inside a
 Distribution
 */
class CombinationFinder {
  friend class DistributionConcrete;

public:
  /**
   * @brief searches for matches. For example assume having built this
   object with a containingGroup equal to <A,B,C,D> and the variables
   describing the domain
   * of the reference distribution equal to <B,D>. When passing comb as
   <0,1,2,0>, it searches for the <combination,image> pertaining to
   this combination
   * <B,D> = <1,0>.
   * @param the combination to search, referring to the set of
   variables passed when building this object.
   * @return the pair <combination,image> of the the matching
   combination. <nullptr,0> is returned in case such a combination was
   not explicitly put in the distribution.
   */
  struct Result {
    CombinationRawValuesMap::const_iterator map_iterator;
    float value;
  };
  Result find(const categoric::Combination &comb) const;

private:
  CombinationFinder(const EvaluatorPtr &evaluator,
                    const CombinationRawValuesMapPtr &combinations_map,
                    const categoric::VariablesSoup &distribution_group,
                    const categoric::VariablesSoup &bigger_group);

  EvaluatorPtr evaluator;
  CombinationRawValuesMapPtr combinations_map;

  const std::vector<std::size_t> indices_in_bigger_group;
};
} // namespace EFG::distribution
