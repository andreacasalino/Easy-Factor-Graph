/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Distribution.h>
#include <EasyFactorGraph/structure/Node.h>

#include <unordered_map>

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
class NodesAware {
public:
  virtual ~NodesAware() = default;

  const std::unordered_set<EFG::distribution::DistributionCnstPtr> &
  getAllFactors() const {
    return this->factorsAll;
  };

  /**
   * @return all the variables (hidden or observed) in the model
   */
  categoric::VariablesSet getVariables() const;

  categoric::VariablePtr findVariable(const std::string &name) const;

protected:
  NodesAware() = default;

  void
  addDistribution(const EFG::distribution::DistributionCnstPtr &distribution);

  Node *findNode(const std::string &name) const;

  const std::unordered_map<categoric::VariablePtr, Node> &getNodes() const {
    return nodes;
  };

private:
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
};
} // namespace EFG::strct
