/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/Node.h>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace EFG::strct {
using Evidences = std::unordered_map<categoric::VariablePtr, std::size_t>;

class EvidencesAware {
public:
  virtual ~EvidencesAware() = default;

  std::unordered_set<categoric::VariablePtr> getHiddenVariables() const;
  std::unordered_set<categoric::VariablePtr> getObservedVariables() const;
  const Evidences &getEvidences() const { return context.evidences; };

protected:
  EvidencesAware() = default;

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

  const EvidenceContext &getEvidenceContext() const { return context; };

  void setEvidence(const categoric::VariablePtr &var, const std::size_t value,
                   const bool evidence_can_be_created = true);

  void resetEvidences();

  void addHidden(Node *node);

private:
  EvidenceContext context;
};
} // namespace EFG::strct
