/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/structure/FactorsManager.h>

namespace EFG::strct {
void FactorsAdder::addConstFactor(
    const distribution::DistributionCnstPtr &factor) {
  addDistribution(factor);
  const_factors.emplace(factor);
}

void FactorsAdder::copyConstFactor(const distribution::Distribution &factor) {
  auto cloned = std::make_shared<distribution::Factor>(factor);
  addConstFactor(cloned);
}
} // namespace EFG::strct
