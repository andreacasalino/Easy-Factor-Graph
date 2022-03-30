/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/components/BeliefAware.h>
#include <EasyFactorGraph/structure/components/StateAware.h>

namespace EFG::strct {
template <typename T> struct SharedPtrHasher {
  std::size_t operator()(const std::shared_ptr<T> &subject) const {
    return std::hash<const T *>{}(subject.get());
  }
};

template <typename T> struct SharedPtrComparator {
  std::size_t operator()(const std::shared_ptr<T> &a,
                         const std::shared_ptr<T> &b) const {
    return a.get() == b.get();
  }
};

template <typename T>
using UnorderedSet = std::unordered_set<std::shared_ptr<T>, SharedPtrHasher<T>,
                                        SharedPtrComparator<T>>;

class ConnectionsManager : virtual public StateAware,
                           virtual public BeliefAware {
public:
  const UnorderedSet<const distribution::Distribution> &getAllFactors() const {
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
  UnorderedSet<const distribution::Distribution> factorsAll;
};
} // namespace EFG::strct
