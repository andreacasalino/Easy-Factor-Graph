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
categoric::VariablesSet StateAware::getVariables() const {
  categoric::VariablesSet result;
  for (const auto &[var, el] : getState().nodes) {
    result.emplace(var);
  }
  return result;
}

categoric::VariablePtr StateAware::findVariable(const std::string &name) const {
  const auto &nodes = getState().nodes;
  auto nodes_it = nodes.find(categoric::make_variable(2, name));
  if (nodes_it == nodes.end()) {
    throw Error{name, " is an inexistent variable"};
  }
  return nodes_it->first;
}

categoric::VariablesSet StateAware::getAllVariables() const {
  auto result = getHiddenVariables();
  auto obs = getObservedVariables();
  result.insert(obs.begin(), obs.end());
  return result;
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
    throw Error{variable->name(), " is non existing in this graph"};
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
  if (clusters_it != state.clusters.end()) {
    return HiddenNodeLocation{clusters_it, node};
  }
  return std::nullopt;
}
} // namespace EFG::strct
