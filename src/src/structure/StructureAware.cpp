/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/distribution/Factor.h>
#include <EasyFactorGraph/structure/StructureAware.h>

#include <variant>

namespace EFG::strct {
categoric::VariablesSet StructureAware::getVariables() const {
  categoric::VariablesSet result;
  for (const auto &[var, node] : nodes) {
    result.emplace(var);
  }
  return result;
}

std::unordered_set<categoric::VariablePtr>
StructureAware::getHiddenVariables() const {
  std::unordered_set<categoric::VariablePtr> result;
  for (const auto &cluster : context.hidden_clusters) {
    for (const auto *var : cluster) {
      result.emplace(var->variable);
    }
  }
  return result;
}

std::unordered_set<categoric::VariablePtr>
StructureAware::getObservedVariables() const {
  std::unordered_set<categoric::VariablePtr> result;
  for (const auto &[var, evidence] : context.evidences) {
    result.emplace(var);
  }
  return result;
}

categoric::VariablePtr
StructureAware::findVariable(const std::string &name) const {
  auto nodes_it = nodes.find(categoric::make_variable(2, name));
  if (nodes_it == nodes.end()) {
    throw Error{name, " is an inexistent variable"};
  }
  return nodes_it->first;
}

Node *StructureAware::findOrMakeNode(const categoric::VariablePtr &var) {
  auto nodes_it = nodes.find(var);
  if (nodes_it == nodes.end()) {
    nodes_it = nodes.emplace(var, Node{}).first;
    context.hidden_clusters.emplace_back().emplace(&nodes_it->second);
  } else if (nodes_it->first.get() != var.get()) {
    throw Error{"Trying to insert variable named: ", var->name(),
                " multiple times"};
  }
  return &nodes_it->second;
}

void StructureAware::addUnaryDistribution(
    const EFG::distribution::DistributionCnstPtr &unary_factor) {
  const auto &vars = unary_factor->getVariables().getVariables();
  auto nodeA = findOrMakeNode(vars.front());
  nodeA->unaryFactors.push_back(unary_factor);
  factorsAll.emplace(unary_factor);
}

namespace {
void connect(const EFG::distribution::DistributionCnstPtr &binary_factor,
             Node *A, Node *B) {
  if (A->activeConnections.find(B) != A->activeConnections.end()) {
    throw Error{A->variable->name(), " and ", B->variable->name(),
                " are already connected with a factor"};
  }
  if (A->disabledConnections.find(B) != A->disabledConnections.end()) {
    throw Error{A->variable->name(), " and ", B->variable->name(),
                " are already connected with a factor"};
  }
  A->activeConnections.emplace(B, Connection{binary_factor, nullptr});
  B->activeConnections.emplace(A, Connection{binary_factor, nullptr});
}

using NodeInfo =
    std::variant<std::vector<HiddenCluster>::iterator, Evidences::iterator>;

NodeInfo find_node(EvidenceContext &context, Node &to_find) {
  auto hidden_clusters_it = std::find_if(
      context.hidden_clusters.begin(), context.hidden_clusters.end(),
      [&to_find](HiddenCluster &cluster) {
        return cluster.find(&to_find) != cluster.end();
      });
  if (hidden_clusters_it != context.hidden_clusters.end()) {
    return hidden_clusters_it;
  }
  return context.evidences.find(to_find.variable);
}

std::map<Node *, Connection>::iterator move_to_disabled(Node &subject,
                                                        Node &to_disable) {
  auto active_it = subject.activeConnections.find(&to_disable);
  auto disable_it =
      subject.disabledConnections
          .emplace(&to_disable, Connection{active_it->second.factor, nullptr})
          .first;
  subject.activeConnections.erase(active_it);
  return disable_it;
}

std::unique_ptr<const distribution::Distribution>
make_evidence_message(const distribution::DistributionCnstPtr &binary_factor,
                      const categoric::VariablePtr &evidence_var,
                      const std::size_t evidence) {
  distribution::CombinationRawValuesMapPtr map;
  std::size_t leaving = 1;
  std::size_t remaining = 0;
  if (evidence_var == binary_factor->getVariables().getVariables().front()) {
    std::swap(leaving, remaining);
  }
  const auto &evaluator = binary_factor->getEvaluator();
  for (const auto &[comb, val] : binary_factor->getCombinationsMap()) {
    if (comb.data()[leaving] == evidence) {
      map->emplace(std::vector<std::size_t>{comb.data()[remaining]},
                   evaluator.evaluate(val));
    }
  }

  categoric::VariablePtr hidden_var;
  if (binary_factor->getVariables().getVariables().front() == evidence_var) {
    hidden_var = binary_factor->getVariables().getVariables().back();
  } else {
    hidden_var = binary_factor->getVariables().getVariables().front();
  }

  class EvidenceMessage : public distribution::Factor {
  public:
    EvidenceMessage(const categoric::VariablePtr &var,
                    const distribution::CombinationRawValuesMapPtr &map)
        : distribution::Factor(categoric::Group{var}, map){};
  };

  return std::make_unique<EvidenceMessage>(hidden_var, map);
}
} // namespace

void StructureAware::addBinaryDistribution(
    const EFG::distribution::DistributionCnstPtr &binary_factor) {
  const auto &vars = binary_factor->getVariables().getVariables();
  auto nodeA = findOrMakeNode(vars.front());
  auto nodeB = findOrMakeNode(vars.back());

  connect(binary_factor, nodeA, nodeB);
  factorsAll.emplace(binary_factor);

  auto nodeA_info = find_node(context, *nodeA);
  auto nodeB_info = find_node(context, *nodeB);

  auto *nodeA_as_hidden =
      std::get_if<std::vector<HiddenCluster>::iterator>(&nodeA_info);
  auto *nodeB_as_hidden =
      std::get_if<std::vector<HiddenCluster>::iterator>(&nodeB_info);
  if ((nullptr != nodeA_as_hidden) && (nullptr != nodeB_as_hidden)) {
    // merge clusters
    (*nodeA_as_hidden)->emplace(nodeB);
    context.hidden_clusters.erase(*nodeB_as_hidden);
    return;
  }

  auto disabledA_it = move_to_disabled(*nodeA, *nodeB);
  auto disabledB_it = move_to_disabled(*nodeB, *nodeA);
  auto *nodeA_as_evidence = std::get_if<Evidences::iterator>(&nodeA_info);
  auto *nodeB_as_evidence = std::get_if<Evidences::iterator>(&nodeB_info);
  if ((nullptr != nodeA_as_evidence) && (nullptr != nodeB_as_evidence)) {
    return;
  }

  if (nullptr == nodeA_as_hidden) {
    // nodeA is observation, nodeB is hidden
    disabledB_it->second.message2ThisNode = make_evidence_message(
        binary_factor, nodeA->variable, (*nodeA_as_evidence)->second);
    return;
  }

  // nodeB is observation, nodeA is hidden
  disabledA_it->second.message2ThisNode = make_evidence_message(
      binary_factor, nodeB->variable, (*nodeB_as_evidence)->second);
}

void StructureAware::addDistribution(
    const EFG::distribution::DistributionCnstPtr &distribution) {
  if (factorsAll.find(distribution) != factorsAll.end()) {
    throw Error{"Already inserted factor"};
  }
  switch (distribution->getVariables().getVariables().size()) {
  case 1:
    addUnaryDistribution(distribution);
    resetBelief();
    return;
  case 2:
    addBinaryDistribution(distribution);
    resetBelief();
    return;
  default:
    break;
  }
  throw Error{"Factor with invalid number of variables"};
}
} // namespace EFG::strct
