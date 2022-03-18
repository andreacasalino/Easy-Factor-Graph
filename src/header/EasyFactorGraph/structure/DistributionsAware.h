/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Distribution.h>
#include <EasyFactorGraph/structure/ConnectionsAware.h>

#include <unordered_set>

namespace EFG::strct {
class DistributionsAware {
public:
  virtual ~DistributionsAware() = default;

protected:
  DistributionsAware() = default;

  std::unordered_set<distribution::DistributionCnstPtr> distributions;
};

class DistributionsReader : virtual private DistributionsAware {
public:
  const std::unordered_set<distribution::DistributionCnstPtr> &
  getDistributions() const {
    return distributions;
  }
};

class DistributionsInserter : virtual public ConnectionsAware,
                              virtual private DistributionsAware {
public:
  void
  insertDistribution(const distribution::DistributionCnstPtr &distribution);
};
} // namespace EFG::strct
