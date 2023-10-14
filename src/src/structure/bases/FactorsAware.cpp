/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/misc/Visitor.h>
#include <EasyFactorGraph/structure/bases/FactorsAware.h>

#include <algorithm>

namespace EFG::strct {
void FactorsAware::addDistribution(
    const EFG::factor::ImmutablePtr &distribution) {
  if (nullptr == distribution) {
    throw Error{"null distribution can't be add"};
  }
  if (factorsAll.find(distribution) != factorsAll.end()) {
    throw Error{"Already inserted factor"};
  }
  resetBelief();

  switch (distribution->function().vars().getVariables().size()) {
  case 1:
    addUnaryDistribution(distribution);
    break;
  case 2:
    addBinaryDistribution(distribution);
    break;
  default:
    throw Error{"Factor with invalid number of variables"};
    break;
  }

  factorsAll.emplace(distribution);
}

namespace {
void check_is_same_variable(const categoric::VariablePtr &a,
                            const categoric::VariablePtr &b) {
  if (a.get() != b.get()) {
    throw Error::make("Trying to insert variable named: ", a->name(),
                      " multiple times with different VariablePtr");
  }
}
} // namespace

StateAware::NodeLocation
FactorsAware::findOrMakeNode(const categoric::VariablePtr &var) {
  if (auto info = locate(var); info.has_value()) {
    check_is_same_variable(var, info.value().node->variable);
    return *info;
  }
  // create this node
  auto &state = stateMutable();
  state.variables.push_back(var);
  auto *added =
      state.nodes.emplace(var, std::make_unique<Node>()).first->second.get();
  added->variable = var;
  HiddenClusters::iterator res_it;
  state.clusters.emplace_back().nodes.emplace(added);
  res_it = state.clusters.end();
  --res_it;
  return NodeLocation{added, res_it};
}

void FactorsAware::addUnaryDistribution(
    const EFG::factor::ImmutablePtr &unary_factor) {
  const auto &var = unary_factor->function().vars().getVariables().front();
  auto &&[node, _] = findOrMakeNode(var);
  node->unary_factors.push_back(unary_factor);
  node->merged_unaries.reset();
}

namespace {
void hybrid_insertion(Node *node_hidden, Node *node_evidence,
                      std::size_t evidence,
                      const EFG::factor::ImmutablePtr &binary_factor) {
  Node::disable(*node_hidden, *node_evidence, binary_factor).first->message =
      std::make_unique<factor::Evidence>(*binary_factor,
                                         node_evidence->variable, evidence);
  node_hidden->merged_unaries.reset();
};
} // namespace

void FactorsAware::addBinaryDistribution(
    const EFG::factor::ImmutablePtr &binary_factor) {
  const auto &vars = binary_factor->function().vars().getVariables();
  const auto nodeA_location = findOrMakeNode(vars.front());
  auto *nodeA = nodeA_location.node;
  const auto nodeB_location = findOrMakeNode(vars.back());
  auto *nodeB = nodeB_location.node;

  if ((nodeA->active_connections.find(nodeB) !=
       nodeA->active_connections.end()) ||
      (nodeA->disabled_connections.find(nodeB) !=
       nodeA->disabled_connections.end())) {
    throw Error::make(nodeA->variable->name(), " and ", nodeB->variable->name(),
                      " are already connected");
  }

  VisitorConst<HiddenClusters::iterator, Evidences::iterator>{
      [&](const HiddenClusters::iterator &hiddenA_location) {
        VisitorConst<HiddenClusters::iterator, Evidences::iterator>{
            [&](const HiddenClusters::iterator &hiddenB_location) {
              // both are hidden
              Node::activate(*nodeA, *nodeB, binary_factor);
              hiddenA_location->connectivity.reset();
              if (hiddenA_location != hiddenB_location) {
                hiddenA_location->nodes.insert(hiddenB_location->nodes.begin(),
                                               hiddenB_location->nodes.end());
                this->stateMutable().clusters.erase(hiddenB_location);
              }
            },
            [&](const Evidences::iterator &evidenceB_location) {
              hybrid_insertion(nodeA, nodeB, evidenceB_location->second,
                               binary_factor);
            }}
            .visit(nodeB_location.location);
      },
      [&](const Evidences::iterator &evidenceA_location) {
        VisitorConst<HiddenClusters::iterator, Evidences::iterator>{
            [&](const HiddenClusters::iterator &) {
              hybrid_insertion(nodeB, nodeA, evidenceA_location->second,
                               binary_factor);
            },
            [&](const Evidences::iterator &) {
              // both are evidences
              Node::disable(*nodeA, *nodeB, binary_factor);
            }}
            .visit(nodeB_location.location);
      }}
      .visit(nodeA_location.location);
}
} // namespace EFG::strct
