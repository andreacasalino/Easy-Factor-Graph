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
  /**
   * @brief update the evidence set with the specified new evidence.
   * In case the involved variable was already part of the evidence set, the
   * evidence value is simply updated.
   * On the contrary case, the involved variable is moved into the evidence set,
   * with the specified value.
   * @param the involved variable
   * @param the evidence value
   * @throw in case the passed variable is not part of the model.
   */
  void setEvidence(const categoric::VariablePtr &variable,
                   const std::size_t value);
};

class EvidenceRemover : virtual public StateAware, virtual public BeliefAware {
public:
  /**
   * @brief update the evidence set by removing the specified variable.
   * @param the involved variable
   * @throw in case the passed variable is not part of the model.
   * @throw in case the passed variable is not part of the current evidence set.
   */
  void removeEvidence(const categoric::VariablePtr &variable);

  /**
   * @brief update the evidence set by removing all the specified variables.
   * @param the involved variables
   * @throw in case one of the passed variable is not part of the model.
   * @throw in case one of the passed variable is not part of the current
   * evidence set.
   */
  void removeEvidences(const categoric::VariablesSet &variables);

  /**
   * @brief removes all the evidences currently set for this model.
   */
  void removeAllEvidences();

private:
  void removeEvidence_(const categoric::VariablePtr &variable);

  void resetState();
};
} // namespace EFG::strct
