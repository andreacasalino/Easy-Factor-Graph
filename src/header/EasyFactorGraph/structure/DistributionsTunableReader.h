/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/DistributionsTunableAware.h>

#include <unordered_set>

namespace EFG::strct {
class DistributionsReader : virtual private DistributionsTunableAware {
public:
  const std::unordered_set<FactorExponentialPtr> &
  getTunableDistributions() const {
    return tunable_distributions;
  }
};
} // namespace EFG::strct
