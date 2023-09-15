/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/misc/Cache.h>
#include <EasyFactorGraph/misc/SmartMap.h>
#include <EasyFactorGraph/structure/SpecialFactors.h>

#include <list>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <variant>

namespace EFG::strct {

struct Node {
  categoric::VariablePtr variable;

  struct Connection {
    factor::ImmutablePtr factor;
    // incoming message
    // nullptr when the message is not already available
    std::unique_ptr<const factor::UnaryFactor> message;
  };

  std::unordered_map<Node *, Connection> active_connections;
  // in this casethe incoming message represents the marginalized evidence
  std::unordered_map<Node *, Connection> disabled_connections;

  std::vector<factor::ImmutablePtr> unary_factors;

  Cache<const factor::UnaryFactor>
      merged_unaries; // merged factor containing all the unary factors and the
  // marginalized evidences

  void updateMergedUnaries();

  static std::pair<Connection *, Connection *>
  activate(Node &a, Node &b, factor::ImmutablePtr factor);

  static std::pair<Connection *, Connection *>
  disable(Node &a, Node &b, factor::ImmutablePtr factor = nullptr);
};

using Nodes = SmartMap<categoric::Variable, std::unique_ptr<Node>>;

enum class PropagationKind { SUM, MAP };

/**
 * @brief Clusters of hidden node. Each cluster is a group of
 * connected hidden nodes.
 * Nodes in different clusters are not currently connected, due to
 * the model structure or the kind of evidences currently applied.
 */
struct HiddenCluster {
  std::unordered_set<Node *> nodes;

  struct TopologyInfo {
    Node *sender;
    Node::Connection *connection;
    std::vector<const Node::Connection *> dependencies;

    bool canUpdateMessage() const;

    // throw when the computation is not possible
    // MAX_VARIATION that the message was computed and before was nullopt
    // any other number is the delta w.r.t, the previous message
    std::optional<float> updateMessage(PropagationKind kind);
  };
  Cache<std::vector<TopologyInfo>> connectivity;

  void updateConnectivity();
};

using HiddenClusters = std::list<HiddenCluster>;

HiddenClusters compute_clusters(const std::unordered_set<Node *> &nodes);

using Evidences = SmartMap<categoric::Variable, std::size_t>;

struct PropagationContext {
  /**
   * @brief maximum number of iterations to use when trying to calibrate a loopy
   * graph
   */
  std::size_t max_iterations_loopy_propagation;
};

/**
 * @brief a structure that can be exposed after having propagated the belief,
 * providing info on the encountered structure.
 */
struct PropagationResult {
  PropagationKind propagation_kind_done;
  bool was_completed;

  struct ClusterInfo {
    bool tree_or_loopy_graph;
    /**
     * @brief number of nodes that constitutes the sub-graph cluster.
     */
    std::size_t size;
  };
  std::vector<ClusterInfo> structures;
};

} // namespace EFG::strct
