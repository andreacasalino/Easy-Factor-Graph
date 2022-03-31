/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Distribution.h>
#include <EasyFactorGraph/misc/SmartMap.h>
#include <EasyFactorGraph/structure/SpecialFactors.h>

#include <map>
#include <optional>
#include <set>
#include <variant>

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

using Nodes = SmartMap<categoric::Variable, Node>;

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

struct GraphState {
  Nodes nodes;
  std::vector<HiddenCluster> clusters;
  Evidences evidences;
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

class StateAware {
public:
  virtual ~StateAware() = default;

  categoric::VariablesSet getHiddenVariables() const;
  categoric::VariablesSet getObservedVariables() const;

  const Evidences &getEvidences() const { return getState().evidences; };

  /**
   * @return all the variables (hidden or observed) in the model
   */
  categoric::VariablesSet getVariables() const;

  categoric::VariablePtr findVariable(const std::string &name) const;

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
