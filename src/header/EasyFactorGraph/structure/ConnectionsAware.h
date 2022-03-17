/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Distribution.h>
#include <EasyFactorGraph/structure/BeliefAware.h>
#include <EasyFactorGraph/structure/GraphState.h>

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
class ConnectionsAware : virtual public BeliefAware {
public:
  virtual ~ConnectionsAware() = default;

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
  ConnectionsAware(const GraphStatePtr &state) : state(state){};

  void
  addDistribution(const EFG::distribution::DistributionCnstPtr &distribution);

  const Nodes &getEvidences() const { return state->nodes; };

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

  GraphStatePtr state;
};
} // namespace EFG::strct
