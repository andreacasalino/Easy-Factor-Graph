/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/bases/BeliefAware.h>
#include <EasyFactorGraph/structure/bases/StateAware.h>

namespace EFG::strct {
class FactorsAware : virtual public StateAware, virtual public BeliefAware {
public:
  /**
   * @return all the factors in the model, tunable and constant.
   */
  const auto &getAllFactors() const { return this->factorsAll; };

protected:
  void addDistribution(const EFG::factor::ImmutablePtr &distribution);

private:
  NodeLocation findOrMakeNode(const categoric::VariablePtr &var);

  void addUnaryDistribution(const EFG::factor::ImmutablePtr &unary_factor);

  void addBinaryDistribution(const EFG::factor::ImmutablePtr &binary_factor);

  /**
   * @brief a register storing ALL the factors in the model, no matter
   the kind (exponential, const, non const)
   */
  std::unordered_set<factor::ImmutablePtr> factorsAll;
};
} // namespace EFG::strct
