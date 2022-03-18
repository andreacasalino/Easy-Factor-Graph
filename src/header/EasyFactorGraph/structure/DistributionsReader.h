/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/DistributionsAware.h>

namespace EFG::strct {
class DistributionsReader : virtual private DistributionsAware {
public:
  const std::unordered_set<distribution::DistributionCnstPtr> &
  getDistributions() const {
    return distributions;
  }
};
} // namespace EFG::strct
