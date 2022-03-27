/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Distribution.h>
#include <EasyFactorGraph/structure/SpecialFactors.h>

#include <map>
#include <set>
#include <unordered_map>
#include <variant>
#include <vector>

namespace EFG::strct {
template <typename T> using Cache = std::unique_ptr<T>;

struct Connection;
struct Node {
  categoric::VariablePtr variable;

  // incoming messages
  std::map<Node *, Connection> active_connections;

  std::map<Node *, Connection> disabled_connections;
  std::vector<distribution::DistributionCnstPtr> unary_factors;

  Cache<const distribution::UnaryFactor>
      merged_unaries; // unary factors and marginalized evidences
};

struct Connection {
  // info for message computation
  distribution::DistributionCnstPtr factor;

  // nullptr when the message is not already available
  std::unique_ptr<const distribution::UnaryFactor> message;
};

using Nodes = std::unordered_map<categoric::VariablePtr, Node>;

struct ConnectionAndDependencies {
  Connection *connection;
  Node *sender;
  std::vector<const Connection *> dependencies;
};
/**
 * @brief Clusters of hidden node. Each cluster is a group of
 connected hidden nodes.
 * Nodes in different clusters are not currently connected (due to
 the model structure or the kind of evidences currently set)
 */
struct HiddenCluster {
  std::set<Node *> nodes;
  Cache<std::vector<ConnectionAndDependencies>> connectivity;
};

using Evidences = std::unordered_map<categoric::VariablePtr, std::size_t>;

struct GraphState {
  Nodes nodes;
  std::vector<HiddenCluster> clusters;
  Evidences evidences;
};

class GraphStateAware {
public:
  virtual ~GraphStateAware() = default;

protected:
  GraphStateAware() { state = std::make_shared<GraphState>(); };

  std::shared_ptr<GraphState> state;
};

struct HiddenNodeLocation {
  std::vector<HiddenCluster>::iterator cluster;
  Node *node;
};

struct EvidenceNodeLocation {
  Evidences::iterator evidence;
  Node *node;
};

using NodeLocation = std::variant<HiddenNodeLocation, EvidenceNodeLocation>;
} // namespace EFG::strct
