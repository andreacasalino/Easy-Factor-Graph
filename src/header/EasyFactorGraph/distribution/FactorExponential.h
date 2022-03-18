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

class FactorExponential : public DistributionConcrete,
                          protected DistributionSetter {
public:
  FactorExponential(const Factor &factor);
  FactorExponential(const Factor &factor, const float weigth);

  /**
   * @brief sets the weight used by teh exponential function converting the
   raw
   * images
   */
  void setWeight(float w);
  float getWeight() const;
};
} // namespace EFG::distribution
