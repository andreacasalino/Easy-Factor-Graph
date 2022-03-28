/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/BeliefAware.h>
#include <EasyFactorGraph/structure/GraphState.h>

#include <functional>
#include <limits>
#include <optional>

namespace EFG::strct {
std::vector<HiddenCluster>::iterator
find_cluster(GraphState &state, const categoric::VariablePtr &variable);

std::optional<NodeLocation> find_node(GraphState &state,
                                      const categoric::VariablePtr &variable);

void visit_location(
    const NodeLocation &to_visit,
    std::function<void(const HiddenNodeLocation &)> hidden_case,
    std::function<void(const EvidenceNodeLocation &)> evidence_case);

std::vector<HiddenCluster> compute_clusters(const std::set<Node *> &nodes);

void update_merged_unaries(Node &subject);

void update_connectivity(HiddenCluster &subject);

using MessageVariation = float;

static const MessageVariation MAX_VARIATION = std::numeric_limits<float>::max();

bool can_update_message(const ConnectionAndDependencies &subject);

// throw when the computation is not possible
// MAX_VARIATION that the message was computed and before was nullptr
// any other number is the delta w.r.t, the previous message
std::optional<MessageVariation>
update_message(ConnectionAndDependencies &subject, const PropagationKind &kind);
} // namespace EFG::strct
