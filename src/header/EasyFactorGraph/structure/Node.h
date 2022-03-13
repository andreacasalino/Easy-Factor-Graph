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
// struct Connection {
//   distribution::DistributionPtr factor;
//   // nullptr when the message is not already available
//   std::unique_ptr<distribution::Distribution> message2ThisNode;
// };

// struct Node {
//   explicit Node(categoric::VariablePtr var);

//   categoric::VariablePtr variable;
//   std::list<distribution::DistributionPtr> unaryFactors;
//   std::map<Node *, Connection> activeConnections;
//   // here message to this is the marginalized factor
//   std::map<Node *, Connection> disabledConnections;
// };
} // namespace EFG::strct
