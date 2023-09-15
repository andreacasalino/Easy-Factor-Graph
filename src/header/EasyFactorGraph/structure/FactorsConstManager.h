/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/bases/FactorsAware.h>

namespace EFG::strct {
class FactorsConstGetter : virtual public FactorsAware {
public:
  /**
   * @return the collection of const factors that are part of the model. Tunable
   * factors are not accounted in this collection.
   */
  const auto &getConstFactors() const { return const_factors; }

protected:
  std::unordered_set<factor::ImmutablePtr> const_factors;
};

class FactorsConstInserter : virtual public FactorsConstGetter {
public:
  /**
   * @brief add a shallow copy of the passed const factor to this model
   */
  void addConstFactor(const factor::ImmutablePtr &factor);
  /**
   * @brief add a deep copy of the passed const factor to this model
   */
  void copyConstFactor(const factor::Immutable &factor);

  /**
   * @brief adds a collection of const factors ot this model.
   * Passing copy = true, deep copies are created and inserted in this model.
   * Passing copy = false, shallow copies are created and inserted in this
   * model.
   */
  template <typename DistributionIt>
  void absorbConstFactors(const DistributionIt &begin,
                          const DistributionIt &end, bool copy) {
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
