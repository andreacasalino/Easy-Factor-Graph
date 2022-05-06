/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/ConnectionsManager.h>

namespace EFG::strct {
class FactorsAware : virtual public ConnectionsManager {
public:
  /**
   * @return the collection of const factors that are part of the model. Tunable
   * factors are not accounted in this collection.
   */
  const std::unordered_set<distribution::DistributionCnstPtr> &
  getConstFactors() const {
    return const_factors;
  }

protected:
  std::unordered_set<distribution::DistributionCnstPtr> const_factors;
};

class FactorsAdder : virtual public FactorsAware {
public:
  /**
   * @brief add a shallow copy of the passed const factor to this model
   */
  void addConstFactor(const distribution::DistributionCnstPtr &factor);
  /**
   * @brief add a deep copy of the passed const factor to this model
   */
  void copyConstFactor(const distribution::Distribution &factor);

  /**
   * @brief adds a collection of const factors ot this model.
   * Passing copy = true, deep copies are created and inserted in this model.
   * Passing copy = false, shallow copies are created and inserted in this
   * model.
   */
  template <typename DistributionIt>
  void absorbConstFactors(const DistributionIt &begin,
                          const DistributionIt &end, const bool copy) {
    for (auto it = begin; it != end; ++it) {
      try {
        if (copy) {
          copyConstFactor(**it);
        } else {
          addConstFactor(*it);
        }
      } catch (...) {
      }
    }
  }
};
} // namespace EFG::strct
