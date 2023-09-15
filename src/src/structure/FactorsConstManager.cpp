/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/structure/FactorsConstManager.h>

namespace EFG::strct {
void FactorsConstInserter::addConstFactor(const factor::ImmutablePtr &factor) {
  addDistribution(factor);
  const_factors.emplace(factor);
}

void FactorsConstInserter::copyConstFactor(const factor::Immutable &factor) {
  auto cloned = std::make_shared<factor::Factor>(
      factor, factor::Factor::CloneTrasformedImagesTag{});

  addConstFactor(cloned);
}
} // namespace EFG::strct
