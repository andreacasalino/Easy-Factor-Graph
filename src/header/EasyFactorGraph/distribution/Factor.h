/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/DistributionConcrete.h>
#include <EasyFactorGraph/distribution/DistributionSetter.h>

namespace EFG::distribution {
class BasicEvaluator;

class Factor : public DistributionConcrete, public DistributionSetter {
public:
  Factor(const categoric::Group &vars);
};
} // namespace EFG::distribution
