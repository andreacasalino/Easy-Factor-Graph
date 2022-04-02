/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/components/StateAware.h>

#include <algorithm>

namespace EFG::strct {
categoric::VariablePtr StateAware::findVariable(const std::string &name) const {
  auto variables_it =
      std::find_if(state.variables.begin(), state.variables.end(),
                   [&name](const categoric::VariablePtr &var) {
                     return var->name() == name;
                   });
  if (variables_it == state.variables.end()) {
    throw Error{name, " is an inexistent variable"};
  }
  return *variables_it;
}

categoric::VariablesSet StateAware::getHiddenVariables() const {
  categoric::VariablesSet result;
  for (const auto &cluster : getState().clusters) {
    for (auto *node : cluster.nodes) {
      result.emplace(node->variable);
    }
  }
  return result;
}

categoric::VariablesSet StateAware::getObservedVariables() const {
  categoric::VariablesSet result;
  for (const auto &[var, val] : getState().evidences) {
    result.emplace(var);
  }
  return result;
}

std::optional<NodeLocation>
StateAware::locate(const categoric::VariablePtr &variable) const {
  Node *node = nullptr;
  {
    auto nodes_it = state.nodes.find(variable);
    if (nodes_it != state.nodes.end()) {
      node = &nodes_it->second;
    }
  }
  if (nullptr == node) {
    return std::nullopt;
  }
  auto evidences_it = state.evidences.find(variable);
  if (evidences_it != state.evidences.end()) {
    return EvidenceNodeLocation{evidences_it, node};
  }
  auto clusters_it =
      std::find_if(state.clusters.begin(), state.clusters.end(),
                   [&node](const HiddenCluster &element) {
                     return element.nodes.find(node) != element.nodes.end();
                   });
  return HiddenNodeLocation{clusters_it, node};
}
} // namespace EFG::strct
