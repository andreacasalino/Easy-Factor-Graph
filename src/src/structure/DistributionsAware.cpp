/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/structure/DistributionsAware.h>

namespace EFG::strct {
void DistributionsInserter::insertDistribution(
    const distribution::DistributionCnstPtr &distribution) {
  addDistribution(distribution);
  distributions.emplace(distribution);
}
} // namespace EFG::strct
