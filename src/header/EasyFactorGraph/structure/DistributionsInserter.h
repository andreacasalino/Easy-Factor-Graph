/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/ConnectionsAware.h>
#include <EasyFactorGraph/structure/DistributionsAware.h>

namespace EFG::strct {
class DistributionsInserter : virtual public ConnectionsAware,
                              virtual private DistributionsAware {
public:
  void
  insertDistribution(const distribution::DistributionCnstPtr &distribution);
};
} // namespace EFG::strct
