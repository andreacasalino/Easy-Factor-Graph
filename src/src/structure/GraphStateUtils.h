/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/GraphState.h>

#include <variant>

namespace EFG::strct {
using NodeInfo =
    std::variant<std::vector<HiddenCluster>::iterator, Evidences::iterator>;

NodeInfo find_node(GraphState &state, Node &to_find);

std::map<Node *, Connection>::iterator move_to_disabled(Node &subject,
                                                        Node &to_disable);

std::map<Node *, Connection>::iterator move_to_enabled(Node &subject,
                                                       Node &to_enable);

std::unique_ptr<const distribution::Distribution>
make_evidence_message(const distribution::DistributionCnstPtr &binary_factor,
                      const categoric::VariablePtr &evidence_var,
                      const std::size_t evidence);
} // namespace EFG::strct
