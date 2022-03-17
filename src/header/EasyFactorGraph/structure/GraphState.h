/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/Node.h>

#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

namespace EFG::strct {
using Evidences = std::unordered_map<categoric::VariablePtr, std::size_t>;

/**
 * @brief Clusters of hidden node. Each cluster is a group of
 connected hidden nodes.
 * Nodes in different clusters are not currently connected (due to
 the model structure or the kind of evidences currently set)
 */
using HiddenCluster = std::set<Node *>;
struct EvidenceContext {
  std::vector<HiddenCluster> hidden_clusters;
  Evidences evidences;
};

/**
 * @brief The set of variables part of the model, with the
 connectivity information
 */
using Nodes = std::unordered_map<categoric::VariablePtr, Node>;

struct GraphState {
  Evidences evidences;
  std::vector<HiddenCluster> hidden_clusters;
  Nodes nodes;
};

class GraphStateAware {
public:
  virtual ~GraphStateAware() = default;

protected:
  GraphStateAware() { state = std::make_shared<GraphState>(); };

  std::shared_ptr<GraphState> state;
};
} // namespace EFG::strct
