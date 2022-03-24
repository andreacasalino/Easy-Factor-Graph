/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/GraphState.h>

namespace EFG::strct {
distribution::DistributionCnstPtr merge_unary_factors(
    const std::vector<distribution::DistributionCnstPtr> &factors);

using MessageVariation = float;

constexpr MessageVariation MAX_VARIATION = std::numeric_limits<float>::max();

MessageVariation update_message(const MessageAndDependencies &subject);

void update_merged_contributions(Node &subject);

void update_connectivity(HiddenCluster &subject);
} // namespace EFG::strct
