/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Distribution.h>
#include <EasyFactorGraph/structure/BeliefAware.h>
#include <EasyFactorGraph/structure/Node.h>

#include <set>
#include <unordered_map>
#include <unordered_set>

namespace std {
template <> class hash<EFG::distribution::DistributionCnstPtr> {
public:
  std::size_t
  operator()(const EFG::distribution::DistributionCnstPtr &subject) const {
    return std::hash<const EFG::distribution::Distribution *>{}(subject.get());
  };
};
} // namespace std

namespace EFG::strct {
using Evidences = std::unordered_map<categoric::VariablePtr, std::size_t>;

/**
 * @brief Clusters of hidden node. Each cluster is a group of
 connected hidden nodes.
 * Nodes in different clusters are not currently connected (due to
 the model structure or the kind of evidences currently set)
 */
using HiddenCluster = std::set<Node *>;
struct EvidenceContext {
  std::vector<HiddenCluster> hidden_clusters;
  Evidences evidences;
};

class StructureAware : virtual public BeliefAware {
public:
  virtual ~StructureAware() = default;

  const std::unordered_set<EFG::distribution::DistributionCnstPtr> &
  getAllFactors() const {
    return this->factorsAll;
  };

  /**
   * @return all the variables (hidden or observed) in the model
   */
  categoric::VariablesSet getVariables() const;

  std::unordered_set<categoric::VariablePtr> getHiddenVariables() const;
  std::unordered_set<categoric::VariablePtr> getObservedVariables() const;
  const Evidences &getEvidences() const { return context.evidences; };

  categoric::VariablePtr findVariable(const std::string &name) const;

protected:
  StructureAware() = default;

  const std::unordered_map<categoric::VariablePtr, Node> &getNodes() const {
    return nodes;
  };

  void
  addDistribution(const EFG::distribution::DistributionCnstPtr &distribution);

  const EvidenceContext &getEvidenceContext() const { return context; };

  void setEvidence(Node *node, const std::size_t value,
                   const bool evidence_can_be_created = true);

  void resetEvidence(Node *node);

  void resetEvidences();

private:
  Node *findOrMakeNode(const categoric::VariablePtr &var);

  void addUnaryDistribution(
      const EFG::distribution::DistributionCnstPtr &unary_factor);

  void addBinaryDistribution(
      const EFG::distribution::DistributionCnstPtr &binary_factor);

  /**
   * @brief a register storing ALL the factors in the model, no matter
   the kind (exponential, const, non const)
   */
  std::unordered_set<EFG::distribution::DistributionCnstPtr> factorsAll;

  /**
   * @brief The set of variables part of the model, with the
   connectivity information
   */
  std::unordered_map<categoric::VariablePtr, Node> nodes;

  EvidenceContext context;
};
} // namespace EFG::strct
