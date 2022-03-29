/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/components/BeliefAware.h>
#include <EasyFactorGraph/structure/components/StateAware.h>

namespace EFG::strct {
class EvidenceSetter : virtual public StateAware, virtual public BeliefAware {
public:
  // if the variable is not currently and evidence is add to evidence set
  void setEvidence(const categoric::VariablePtr &variable,
                   const std::size_t value);
};

class EvidenceRemover : virtual public StateAware, virtual public BeliefAware {
public:
  void removeEvidence(const categoric::VariablePtr &variable);

  void removeEvidences();
};
} // namespace EFG::strct
