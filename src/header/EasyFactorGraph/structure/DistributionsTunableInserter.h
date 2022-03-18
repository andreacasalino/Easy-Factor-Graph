/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/ConnectionsAware.h>
#include <EasyFactorGraph/structure/DistributionsTunableAware.h>

namespace EFG::strct {
class DistributionsInserter : virtual public ConnectionsAware,
                              virtual private DistributionsTunableAware {
public:
  void
  insertTunableDistribution(const FactorExponentialPtr &tunable_distribution);

  void
  insertTunableDistribution(const FactorExponentialPtr &tunable_distribution,
                            const categoric::Group &group_sharing_weigth);

  /**
 * @return the weights of the tunable clusters. For each cluster only
 1 value is returned, since it is shared among the elements in the
 same cluster.
 */
  std::vector<float> getWeights() const;
};
} // namespace EFG::strct
