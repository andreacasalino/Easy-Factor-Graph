/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Distribution.h>
#include <list>

namespace EFG::strct {
struct Connection {
  distribution::DistributionCnstPtr factor;

  // nullptr when the message is not already available
  std::unique_ptr<const distribution::Distribution> message2ThisNode;
};

struct Node {
  categoric::VariablePtr variable;
  std::list<distribution::DistributionCnstPtr> unaryFactors;
  std::map<Node *, Connection> activeConnections;
  // here message to this is the marginalized factor
  std::map<Node *, Connection> disabledConnections;
};
} // namespace EFG::strct
