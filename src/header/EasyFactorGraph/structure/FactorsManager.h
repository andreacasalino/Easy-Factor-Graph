/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/ConnectionsManager.h>

namespace EFG::strct {
class FactorsAware : virtual public ConnectionsManager {
public:
  const std::unordered_set<distribution::DistributionCnstPtr> &
  getConstFactors() const {
    return const_factors;
  }

protected:
  std::unordered_set<distribution::DistributionCnstPtr> const_factors;
};

class FactorsAdder : virtual public FactorsAware {
public:
  void addConstFactor(const distribution::DistributionCnstPtr &factor);
  void copyConstFactor(const distribution::Distribution &factor);
};
} // namespace EFG::strct
