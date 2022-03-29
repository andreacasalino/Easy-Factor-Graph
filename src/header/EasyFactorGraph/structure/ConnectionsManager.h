/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/components/BeliefAware.h>
#include <EasyFactorGraph/structure/components/StateAware.h>

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
class ConnectionsManager : virtual public StateAware,
                           virtual public BeliefAware {
public:
  const std::unordered_set<EFG::distribution::DistributionCnstPtr> &
  getAllFactors() const {
    return this->factorsAll;
  };

protected:
  void
  addDistribution(const EFG::distribution::DistributionCnstPtr &distribution);

private:
  NodeLocation findOrMakeNode(const categoric::VariablePtr &var);

  void addUnaryDistribution(
      const EFG::distribution::DistributionCnstPtr &unary_factor);

  void addBinaryDistribution(
      const EFG::distribution::DistributionCnstPtr &binary_factor);

  /**
   * @brief a register storing ALL the factors in the model, no matter
   the kind (exponential, const, non const)
   */
  std::unordered_set<EFG::distribution::DistributionCnstPtr> factorsAll;
};
} // namespace EFG::strct
