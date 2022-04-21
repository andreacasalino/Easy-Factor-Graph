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
 * @brief An object used to search for the images associated to
 * sub combinations that are part of a bigger one.
 */
class CombinationFinder {
  friend class DistributionConcrete;

public:
  /**
   * @brief Searches for matches. For example assume having built this
   * object with a bigger_group equal to <A,B,C,D> while the variables
   * describing the distribution this finder refers to is equal to <B,D>.
   * When passing a comb equal to <0,1,2,0>, this object searches for the
   * immage associated to the sub combination <B,D> = <1,0>.
   * @param the combination of values referring to the bigger_group, which
   * contains the sub combination to search.
   * @return an object storing the sub combination (in case it is explicitly
   * instanciated, otherwise an end iterator is returned) as well as the image
   * associated to it.
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
