/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/misc/Visitor.h>
#include <EasyFactorGraph/structure/EvidenceManager.h>
#include <EasyFactorGraph/structure/bases/StateAware.h>

#include <algorithm>

namespace EFG::strct {
void EvidenceSetter::setEvidence(const categoric::VariablePtr &variable,
                                 std::size_t value) {
  if (variable->size() <= value) {
    throw Error::make(std::to_string(value),
                      " is an invalid evidence for variable ",
                      variable->name());
  }
  auto info = locate(variable);
  if (!info.has_value()) {
    throw Error::make(variable->name(), " is a non existing variable");
  }
  auto *node = info->node;
  Evidences::iterator evidence_location;
  VisitorConst<HiddenClusters::iterator, Evidences::iterator>{
      [&](const HiddenClusters::iterator &it) {
        while (!node->active_connections.empty()) {
          Node::disable(*node, *node->active_connections.begin()->first);
        }
        // update clusters
        auto &state = stateMutable();
        if (1 == it->nodes.size()) {
          state.clusters.erase(it);
        } else {
          auto nodes = it->nodes;
          state.clusters.erase(it);
          nodes.erase(node);
          for (auto &&cluster : compute_clusters(nodes)) {
            state.clusters.emplace_back(std::move(cluster));
          }
        }
        evidence_location =
            state.evidences.emplace(node->variable, value).first;
      },
      [&](const Evidences::iterator &it) {
        evidence_location = it;
        evidence_location->second = value;
      }}
      .visit(info->location);

  for (auto &[connected_node, connection] : node->disabled_connections) {
    auto connection_it = connected_node->disabled_connections.find(node);
    connection_it->second.message = std::make_unique<factor::Evidence>(
        *connection_it->second.factor, node->variable,
        evidence_location->second);
    connected_node->merged_unaries.reset();
  }
  resetBelief();
}

void EvidenceSetter::setEvidence(const std::string &variable,
                                 std::size_t value) {
  setEvidence(findVariable(variable), value);
}

void EvidenceRemover::removeEvidence_(const categoric::VariablePtr &variable) {
  auto &state = stateMutable();
  auto evidence_it = state.evidences.find(variable);
  if (evidence_it == state.evidences.end()) {
    throw Error::make(variable->name(), " is not an evidence");
  }
  resetBelief();
  state.evidences.erase(evidence_it);
  auto &node = *state.nodes[variable].get();
  while (!node.disabled_connections.empty()) {
    auto it = node.disabled_connections.begin();
    it->first->merged_unaries.reset();
    Node::activate(node, *it->first, it->second.factor);
  }
  node.merged_unaries.reset();
}

void EvidenceRemover::removeEvidence(const categoric::VariablePtr &variable) {
  removeEvidence_(variable);
  resetState();
}

void EvidenceRemover::removeEvidence(const std::string &variable) {
  removeEvidence(findVariable(variable));
}

void EvidenceRemover::removeEvidences(
    const categoric::VariablesSet &variables) {
  if (variables.empty()) {
    return;
  }
  for (const auto &variable : variables) {
    removeEvidence_(variable);
  }
  resetState();
}

void EvidenceRemover::removeEvidences(
    const std::unordered_set<std::string> &variables) {
  categoric::VariablesSet vars;
  for (const auto &name : variables) {
    vars.emplace(findVariable(name));
  }
  removeEvidences(vars);
}

void EvidenceRemover::removeAllEvidences() {
  const auto &state = this->state();
  while (!state.evidences.empty()) {
    auto var = state.evidences.begin()->first;
    removeEvidence_(var);
  }
  resetState();
}

void EvidenceRemover::resetState() {
  auto &state = stateMutable();
  std::unordered_set<Node *> nodes;
  for (auto &[var, node] : state.nodes) {
    if (state.evidences.find(var) == state.evidences.end()) {
      nodes.emplace(node.get());
    }
  }
  state.clusters = compute_clusters(nodes);
}
} // namespace EFG::strct
