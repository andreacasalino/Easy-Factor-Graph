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
 * @brief Base object for any kind of distribution.
 * Any kind of distribution has:
 *  - A group of variables the distribution refer to
 *  - A domain, represented by the combinations map. To each key in the map, a
 * raw image value (a float number) is associated.
 *  - Images set, which are the image values associated to each element in the
 * combinations map. They can be obtained by applying a certain function f(x) to
 * the raw images.
 * In order to save memory, the combinations having an image equal to 0 are not
 * explicitly instanciated in the combinations map, even if they are accounted
 * when calling evaluate(...)
 */
class Distribution {
public:
  virtual ~Distribution() = default;

  /**
   * @return the evaluator used to compute the images
   */
  virtual const Evaluator &getEvaluator() const = 0;

  virtual const categoric::Group &getGroup() const = 0;
  virtual const CombinationRawValuesMap &getCombinationsMap() const = 0;

  virtual CombinationFinder
  makeFinder(const categoric::VariablesSoup &bigger_group) const = 0;

  /**
   * @brief searches for the image associated to the passed combination
   * @return the value of the image.
   */
  float evaluate(const categoric::Combination &comb) const;

  /**
   * @return the probabilities associated to each combination in the domain,
   * when assuming only the existance of this distribution. Such probabilities
   * are actually the normalized images. The order of returned values, refer to
   * the combinations that can be iterated by categoric::GroupRange on the
   * variables representing this distribution.
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
