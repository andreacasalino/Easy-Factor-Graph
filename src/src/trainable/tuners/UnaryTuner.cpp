/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/structure/SpecialFactors.h>
#include <EasyFactorGraph/trainable/tuners/UnaryTuner.h>

namespace EFG::train {
UnaryTuner::UnaryTuner(
    strct::Node &node,
    const std::shared_ptr<distribution::FactorExponential> &factor,
    const categoric::VariablesSoup &variables_in_model)
    : BaseTuner(factor, variables_in_model), node(node) {}

float UnaryTuner::getGradientBeta() {
  std::vector<const distribution::Distribution *> unaries = {
      node.merged_unaries.get()};
  for (const auto &[connected_node, connection] : node.active_connections) {
    unaries.push_back(connection.message.get());
  }
  distribution::UnaryFactor merged(unaries);
  return dotProduct(merged.getProbabilities());
}
} // namespace EFG::train
