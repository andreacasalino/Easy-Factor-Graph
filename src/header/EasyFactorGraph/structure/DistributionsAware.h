/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Distribution.h>

#include <unordered_set>

namespace EFG::strct {
class DistributionsAware {
public:
  virtual ~DistributionsAware() = default;

protected:
  std::unordered_set<distribution::DistributionCnstPtr> distributions;
};
} // namespace EFG::strct
