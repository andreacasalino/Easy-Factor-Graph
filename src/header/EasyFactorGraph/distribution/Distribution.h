/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/categoric/Combination.h>
#include <EasyFactorGraph/categoric/Group.h>
#include <EasyFactorGraph/distribution/Evaluator.h>
#include <map>

namespace EFG::distribution {
using CombinationRawValuesMap = std::map<categoric::Combination, float>;

class CombinationFinder;

/**
 * @brief Base object for any kind of categoric distribution.
 * Any kind of categoric distribution has:
 *  - A domain, represented by the combinations in the joint domain of the Group
 * associated to this distribution
 *  - Raw images set, which are positive values associated to each element in
 * the domain
 *  - Images set, which are the image values associated to each element in the
 * domain. They can be obtained by applying a certain function f(x) to the raw
 * images In order to save memory, the combinations having an image equal to 0
 * are not explicitly saved even if they are accounted for the opreations
 * involving this distribution.
 */
class Distribution {
public:
  virtual ~Distribution() = default;

  virtual const Evaluator &getEvaluator() const = 0;
  virtual const categoric::Group &getVariables() const = 0;
  virtual const CombinationRawValuesMap &getCombinationsMap() const = 0;

  /**
   * @param the variables referring to the combinations to search. This
   kind of set should contain the subset of variables
   * describing the domain of distribution
   * @throw if some of the variables describing the distribution domain
   are not contained in containingGroup
   */
  virtual CombinationFinder
  makeFinder(const categoric::VariablesSoup &bigger_group) const = 0;

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

protected:
  virtual CombinationRawValuesMap &getCombinationsMap_() = 0;
  virtual Evaluator &getEvaluator_() = 0;

  Distribution() = default;
};

using DistributionPtr = std::shared_ptr<Distribution>;
using DistributionCnstPtr = std::shared_ptr<const Distribution>;
} // namespace EFG::distribution