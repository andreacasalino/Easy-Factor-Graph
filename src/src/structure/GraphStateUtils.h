/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/GraphState.h>

#include <set>
#include <variant>

namespace EFG::strct {
Evidences::iterator find_evidence(GraphState &state, Node &to_find);

std::vector<HiddenCluster>::iterator find_hidden(GraphState &state,
                                                 Node &to_find);

using NodeInfo =
    std::variant<std::vector<HiddenCluster>::iterator, Evidences::iterator>;

NodeInfo find_node(GraphState &state, Node &to_find);

void disable_connection(Node &nodeA, Node &nodeB);

void enable_connection(Node &nodeA, Node &nodeB);

std::unique_ptr<const distribution::Distribution>
make_evidence_message(const distribution::DistributionCnstPtr &binary_factor,
                      const categoric::VariablePtr &evidence_var,
                      const std::size_t evidence);

struct MessageTask {
  Node *sender;
  distribution::DistributionCnstPtr static_merged_dependencies;
  ConnectionAndDependencies recipient;
};
std::vector<MessageTask> serialize_messages(const HiddenCluster &cluster);

std::vector<HiddenCluster> compute_clusters(const std::set<Node *> &nodes);
} // namespace EFG::strct
