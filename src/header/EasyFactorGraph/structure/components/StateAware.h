/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/distribution/Distribution.h>
#include <EasyFactorGraph/misc/Cache.h>
#include <EasyFactorGraph/misc/SmartMap.h>
#include <EasyFactorGraph/structure/SpecialFactors.h>

#include <list>
#include <map>
#include <optional>
#include <set>
#include <variant>

namespace EFG::strct {

struct Connection;
struct Node {
  categoric::VariablePtr variable;

  // incoming messages
  std::map<Node *, std::unique_ptr<Connection>> active_connections;

  std::map<Node *, std::unique_ptr<Connection>> disabled_connections;
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

using Nodes = SmartMap<categoric::Variable, std::unique_ptr<Node>>;

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

using Evidences = SmartMap<categoric::Variable, std::size_t>;

using HiddenClusters = std::list<HiddenCluster>;

struct GraphState {
  categoric::VariablesSoup variables;
  Nodes nodes;
  HiddenClusters clusters;
  Evidences evidences;
};

struct HiddenNodeLocation {
  HiddenClusters::iterator cluster;
  Node *node;
};

struct EvidenceNodeLocation {
  Evidences::iterator evidence;
  Node *node;
};

using NodeLocation = std::variant<HiddenNodeLocation, EvidenceNodeLocation>;

class StateAware {
public:
  virtual ~StateAware() = default;

  const categoric::VariablesSoup &getAllVariables() const {
    return state.variables;
  }

  categoric::VariablesSet getHiddenVariables() const;
  categoric::VariablesSet getObservedVariables() const;

  const Evidences &getEvidences() const { return getState().evidences; };

  categoric::VariablePtr findVariable(const std::string &name) const;

  StateAware(const StateAware &) = delete;
  StateAware &operator=(const StateAware &) = delete;
  StateAware(StateAware &&) = delete;
  StateAware &operator=(StateAware &&) = delete;

protected:
  StateAware() = default;

  const GraphState &getState() const { return state; }
  GraphState &getState_() { return state; }

  std::optional<NodeLocation> locate(const categoric::VariablePtr &var) const;
  std::optional<NodeLocation> locate(const std::string &var_name) const {
    return locate(findVariable(var_name));
  }

private:
  mutable GraphState state;
};
} // namespace EFG::strct
