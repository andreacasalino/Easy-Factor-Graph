/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/BeliefAware.h>
#include <EasyFactorGraph/structure/GraphState.h>

namespace std {
template <> class hash<EFG::distribution::DistributionCnstPtr> {
public:
  std::size_t
  operator()(const EFG::distribution::DistributionCnstPtr &subject) const {
    return std::hash<const EFG::distribution::Distribution *>{}(subject.get());
  };
};
} // namespace std

namespace EFG::strct {
class EvidenceAware : virtual public BeliefAware,
                      virtual private GraphStateAware {
public:
  virtual ~EvidenceAware() = default;

  std::unordered_set<categoric::VariablePtr> getHiddenVariables() const;
  std::unordered_set<categoric::VariablePtr> getObservedVariables() const;

  const Evidences &getEvidences() const { return state->evidences; };
  const std::vector<HiddenCluster> &getHiddenClusters() const {
    return state->hidden_clusters;
  };

protected:
  EvidenceAware() = default;

  void setEvidence(Node *node, const std::size_t value,
                   const bool evidence_can_be_created = true);

  void resetEvidence(Node *node);

  void resetEvidences();
};
} // namespace EFG::strct
