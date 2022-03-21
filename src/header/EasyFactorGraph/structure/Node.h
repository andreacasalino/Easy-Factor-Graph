/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Distribution.h>
#include <list>
#include <map>

namespace EFG::strct {
struct Node;

class Connection {
public:
  // this nullify the dependencies
  Connection(Node &sender_node, Node &receiver_node);

  // this nullify the dependencies
  Connection(Connection &&o);
  Connection &operator=(Connection &&o) = delete;

  const distribution::DistributionCnstPtr factor;

  const distribution::Distribution *getMessage() const {
    return message2ThisNode.get();
  }

  using Dependencies = std::vector<distribution::DistributionCnstPtr>;
  const Dependencies &getDependencies() const;

  bool isUpdateMessagePossible() const;
  float updateMessage() const;

private:
  Node &sender;

  // nullptr when the message is not already available
  distribution::DistributionCnstPtr message2ThisNode;

  std::unique_ptr<Dependencies> proxy_dependencies;
};

using Connections = std::map<Node *, Connection>;

struct Node {
  categoric::VariablePtr variable;
  std::list<distribution::DistributionCnstPtr> unaryFactors;
  Connections activeConnections;
  // here message to this is the marginalized factor
  Connections disabledConnections;
};
} // namespace EFG::strct
