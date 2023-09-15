/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include "HiddenObservedTuner.h"

namespace EFG::train {
HiddenObservedTuner::HiddenObservedTuner(
    strct::Node &nodeHidden, const strct::Evidences::const_iterator &evidence,
    const std::shared_ptr<factor::FactorExponential> &factor,
    const categoric::VariablesSoup &variables_in_model)
    : BaseTuner(factor, variables_in_model), nodeHidden(nodeHidden),
      evidence(evidence) {
  pos_in_factor_hidden = 0;
  pos_in_factor_evidence = 1;
  if (factor->function().vars().getVariables().front().get() ==
      evidence->first.get()) {
    std::swap(pos_in_factor_hidden, pos_in_factor_evidence);
  }
}

float HiddenObservedTuner::getGradientBeta() {
  std::vector<const factor::Immutable *> hidden_unaries = {
      nodeHidden.merged_unaries.get()};
  for (const auto &[connected_node, connection] :
       nodeHidden.active_connections) {
    hidden_unaries.push_back(connection.message.get());
  }
  auto hidden_probs = factor::MergedUnaries{hidden_unaries}.getProbabilities();
  float result = 0;
  const auto &factor_map = getFactor().function();
  std::vector<std::size_t> comb;
  comb.resize(2);
  comb[pos_in_factor_evidence] = evidence->second;
  for (std::size_t h = 0; h < hidden_probs.size(); ++h) {
    comb[pos_in_factor_hidden] = h;
    result += hidden_probs[h] * factor_map.findImage(comb);
  }
  return result;
}
} // namespace EFG::train
