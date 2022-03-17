/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/BeliefAware.h>
#include <EasyFactorGraph/structure/GraphState.h>

namespace EFG::strct {
class EvidenceAware : virtual public BeliefAware,
                      virtual private GraphStateAware {
public:
  virtual ~EvidenceAware() = default;

  std::unordered_set<categoric::VariablePtr> getHiddenVariables() const;
  std::unordered_set<categoric::VariablePtr> getObservedVariables() const;

  const Evidences &getEvidences() const { return state->evidences; };

protected:
  EvidenceAware() = default;

  const std::vector<HiddenCluster> &getHiddenClusters() const {
    return state->hidden_clusters;
  };

  void setEvidence(Node &node, const std::size_t value,
                   const bool observation_should_prexist = true);

  void resetEvidence(Node &node);

  void resetEvidences();
};
} // namespace EFG::strct
