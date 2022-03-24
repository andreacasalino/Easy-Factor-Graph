/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/GraphState.h>

// #include <set>
#include <functional>
#include <optional>

namespace EFG::strct {
std::vector<HiddenCluster>::iterator
find_cluster(GraphState &state, const categoric::VariablePtr &variable);

std::optional<NodeLocation> find_node(GraphState &state,
                                      const categoric::VariablePtr &variable);

void visit(const NodeLocation &to_visit,
           std::function<void(const HiddenNodeLocation &)> hidden_case,
           std::function<void(const EvidenceNodeLocation &)> evidence_case);

distribution::DistributionCnstPtr
make_evidence_message(const distribution::DistributionCnstPtr &binary_factor,
                      const categoric::VariablePtr &evidence_var,
                      const std::size_t evidence);

std::vector<HiddenCluster> compute_clusters(const std::set<Node *> &nodes);
} // namespace EFG::strct
