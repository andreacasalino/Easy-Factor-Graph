/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/distribution/Factor.h>
#include <EasyFactorGraph/structure/DistributionsAware.h>

namespace EFG::strct {
void DistributionsInserter::insertDistribution(
    const distribution::DistributionCnstPtr &distribution) {
  addDistribution(distribution);
  distributions.emplace(distribution);
}

void DistributionsInserter::copyDistribution(
    const distribution::Distribution &distribution) {
  insertDistribution(std::make_shared<distribution::Factor>(distribution));
}
} // namespace EFG::strct
