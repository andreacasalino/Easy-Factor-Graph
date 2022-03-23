/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Distribution.h>

#include <map>
#include <unordered_map>
#include <vector>

namespace EFG::strct {
template <typename T> using Proxy = std::unique_ptr<T>;

struct Connection;
struct HiddenNode {
  categoric::VariablePtr variable;
  distribution::DistributionCnstPtr unary_factors_contribution;
  std::map<HiddenNode *, Connection> incoming_messages;
  std::map<categoric::VariablePtr, distribution::DistributionCnstPtr>
      evidences_contributions;
  Proxy<distribution::DistributionCnstPtr> merged_contributions;
};

struct Connection {
  // nullptr when the message is not already available
  distribution::DistributionCnstPtr message;
  // info for message computation
  distribution::DistributionCnstPtr factor;
};

/**
 * @brief The set of variables part of the model, with the
 connectivity information
 */
using HiddenNodes = std::unordered_map<categoric::VariablePtr, HiddenNode>;

struct EvidenceNode {
  categoric::VariablePtr variable;
  distribution::DistributionCnstPtr unary_factors_contribution;
  std::size_t evidence;
  std::map<categoric::VariablePtr, distribution::DistributionCnstPtr>
      disabled_connections;
};

using Evidences = std::unordered_map<categoric::VariablePtr, EvidenceNode>;

struct ConnectionAndDependencies {
  Connection *connection;
  HiddenNode *sender;
  std::vector<Connection *> dependencies;
};
/**
 * @brief Clusters of hidden node. Each cluster is a group of
 connected hidden nodes.
 * Nodes in different clusters are not currently connected (due to
 the model structure or the kind of evidences currently set)
 */
struct HiddenCluster {
  HiddenNodes nodes;
  Proxy<std::vector<ConnectionAndDependencies>> connectivity;
};

struct GraphState {
  std::vector<HiddenCluster> hidden_clusters;
  Evidences evidences;
};

class GraphStateAware {
public:
  virtual ~GraphStateAware() = default;

protected:
  GraphStateAware() { state = std::make_shared<GraphState>(); };

  std::shared_ptr<GraphState> state;
};
} // namespace EFG::strct
