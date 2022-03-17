/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/distribution/Factor.h>

#include "GraphStateUtils.h"

namespace EFG::strct {
NodeInfo find_node(GraphState &state, Node &to_find) {
  auto hidden_clusters_it =
      std::find_if(state.hidden_clusters.begin(), state.hidden_clusters.end(),
                   [&to_find](HiddenCluster &cluster) {
                     return cluster.find(&to_find) != cluster.end();
                   });
  if (hidden_clusters_it != state.hidden_clusters.end()) {
    return hidden_clusters_it;
  }
  return state.evidences.find(to_find.variable);
}

std::map<Node *, Connection>::iterator move_to_disabled(Node &subject,
                                                        Node &to_disable) {
  auto active_it = subject.activeConnections.find(&to_disable);
  auto disable_it =
      subject.disabledConnections
          .emplace(&to_disable, Connection{active_it->second.factor, nullptr})
          .first;
  subject.activeConnections.erase(active_it);
  return disable_it;
}

std::unique_ptr<const distribution::Distribution>
make_evidence_message(const distribution::DistributionCnstPtr &binary_factor,
                      const categoric::VariablePtr &evidence_var,
                      const std::size_t evidence) {
  distribution::CombinationRawValuesMapPtr map;
  std::size_t leaving = 1;
  std::size_t remaining = 0;
  if (evidence_var == binary_factor->getVariables().getVariables().front()) {
    std::swap(leaving, remaining);
  }
  const auto &evaluator = binary_factor->getEvaluator();
  for (const auto &[comb, val] : binary_factor->getCombinationsMap()) {
    if (comb.data()[leaving] == evidence) {
      map->emplace(std::vector<std::size_t>{comb.data()[remaining]},
                   evaluator.evaluate(val));
    }
  }

  categoric::VariablePtr hidden_var;
  if (binary_factor->getVariables().getVariables().front() == evidence_var) {
    hidden_var = binary_factor->getVariables().getVariables().back();
  } else {
    hidden_var = binary_factor->getVariables().getVariables().front();
  }

  class EvidenceMessage : public distribution::Factor {
  public:
    EvidenceMessage(const categoric::VariablePtr &var,
                    const distribution::CombinationRawValuesMapPtr &map)
        : distribution::Factor(categoric::Group{var}, map){};
  };

  return std::make_unique<EvidenceMessage>(hidden_var, map);
}
} // namespace EFG::strct
