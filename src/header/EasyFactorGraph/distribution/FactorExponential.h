/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/DistributionConcrete.h>
#include <EasyFactorGraph/distribution/DistributionSetter.h>
#include <EasyFactorGraph/distribution/Factor.h>

namespace EFG::distribution {
class ExponentialEvaluator;

/**
 * @brief An exponential factor applies an exponential function to map the raw
 * values inside the combination map to the actual images.
 * More precisely, given the weight w characterizing the factor the image value
 * is obtained in this way:
 * image = exp(w * raw_value)
 *
 * All the combinations are instanciated in the combinations map when building
 * this object.
 */
class FactorExponential : public DistributionConcrete,
                          protected DistributionSetter {
public:
  /**
   * @brief The same variables describing the passed factor are assumed for the
   * object to build.
   * The map of combinations is built by iterating all the possible ones of the
   * group of variables describing the passed factor. The raw values are
   * computed by evaluating the passed factor over each possible combination.
   * @param the baseline factor
   * @param the weight that will be considered by the exponential evaluator
   */
  FactorExponential(const Factor &factor, const float weigth);

  /**
   * @brief Same as FactorExponential(const Factor &, const float), assuming
   * weigth = 1.
   */
  FactorExponential(const Factor &factor);

  FactorExponential(const FactorExponential &o);
  FactorExponential(FactorExponential &&o);

  /**
   * @brief sets the weight used by the exponential evaluator.
   */
  void setWeight(const float w);

  /**
   * @return the weight used by the exponential evaluator.
   */
  float getWeight() const;
};
} // namespace EFG::distribution
