/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Distribution.h>
#include <map>

namespace EFG::strct {
struct Connection {
  distribution::DistributionCnstPtr factor;

  // nullptr when the message is not already available
  distribution::DistributionCnstPtr message2ThisNode;

  std::vector<const distribution::DistributionCnstPtr *> dependencies;
};

struct Node;
using Connections = std::map<Node *, Connection>;

struct Node {
  categoric::VariablePtr variable;
  std::vector<distribution::DistributionCnstPtr> unaryFactors;
  Connections activeConnections;
  // here message to this is the marginalized factor
  Connections disabledConnections;
};
} // namespace EFG::strct
