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

  categoric::VariablesSet getHiddenVariables() const;
  categoric::VariablesSet getObservedVariables() const;

  const Evidences &getEvidences() const { return state->evidences; };

protected:
  EvidenceAware() = default;

  void setEvidence(const categoric::VariablePtr &variable,
                   const std::size_t value,
                   const bool observation_should_prexist = true);

  void removeEvidence(const categoric::VariablePtr &variable);

  void removeEvidences();
};
} // namespace EFG::strct
