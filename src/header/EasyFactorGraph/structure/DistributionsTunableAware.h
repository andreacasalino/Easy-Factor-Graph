/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/FactorExponential.h>

#include <unordered_set>

namespace EFG::strct {
using FactorExponentialPtr = std::shared_ptr<distribution::FactorExponential>;

class DistributionsTunableAware {
public:
  virtual ~DistributionsTunableAware() = default;

protected:
  std::unordered_set<FactorExponentialPtr> tunable_distributions;
};
} // namespace EFG::strct
