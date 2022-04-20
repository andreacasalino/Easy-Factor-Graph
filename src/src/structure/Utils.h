/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/components/BeliefAware.h>
#include <EasyFactorGraph/structure/components/StateAware.h>

#include <functional>
#include <limits>
#include <optional>

namespace EFG::strct {
void visit_location(
    const NodeLocation &to_visit,
    std::function<void(const HiddenNodeLocation &)> hidden_case,
    std::function<void(const EvidenceNodeLocation &)> evidence_case);

HiddenClusters compute_clusters(const std::set<Node *> &nodes);

void activate_connection(Node &a, Node &b,
                         const distribution::DistributionCnstPtr &factor);
void disable_connection(Node &a, Node &b,
                        const distribution::DistributionCnstPtr &factor);

std::vector<const distribution::Distribution *> gather_unaries(Node &subject);

void update_connectivity(HiddenCluster &subject);

using MessageVariation = float;

static const MessageVariation MAX_VARIATION = std::numeric_limits<float>::max();

bool can_update_message(const ConnectionAndDependencies &subject);

// throw when the computation is not possible
// MAX_VARIATION that the message was computed and before was nullptr
// any other number is the delta w.r.t, the previous message
std::optional<MessageVariation>
update_message(ConnectionAndDependencies &subject, const PropagationKind &kind);

std::list<ConnectionAndDependencies *>
pack_all_tasks(std::vector<ConnectionAndDependencies> &tasks);
} // namespace EFG::strct
