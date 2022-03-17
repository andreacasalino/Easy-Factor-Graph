/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/EvidenceAware.h>

namespace EFG::strct {
std::unordered_set<categoric::VariablePtr>
EvidenceAware::getHiddenVariables() const {
  std::unordered_set<categoric::VariablePtr> result;
  for (const auto &cluster : state->hidden_clusters) {
    for (const auto *var : cluster) {
      result.emplace(var->variable);
    }
  }
  return result;
}

std::unordered_set<categoric::VariablePtr>
EvidenceAware::getObservedVariables() const {
  std::unordered_set<categoric::VariablePtr> result;
  for (const auto &[var, evidence] : state->evidences) {
    result.emplace(var);
  }
  return result;
}
} // namespace EFG::strct
