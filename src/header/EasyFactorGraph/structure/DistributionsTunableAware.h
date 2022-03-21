/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/FactorExponential.h>
#include <EasyFactorGraph/structure/ConnectionsAware.h>

namespace EFG::strct {
using FactorExponentialPtr = std::shared_ptr<distribution::FactorExponential>;

using TunableClusters = std::vector<std::vector<FactorExponentialPtr>>;

class DistributionsTunableAware {
public:
  virtual ~DistributionsTunableAware() = default;

protected:
  DistributionsTunableAware() = default;

  TunableClusters tunable_clusters;
};

class DistributionsReader : virtual private DistributionsTunableAware {
public:
  const TunableClusters &getTunableDistributions() const {
    return tunable_clusters;
  }

  /**
   * @return the weights of the tunable clusters. For each cluster only
   * 1 value is returned, since it is shared among the elements in thesame
   * cluster.
   */
  std::vector<float> getWeights() const;
};

class DistributionsTunableInserter : virtual public ConnectionsAware,
                                     virtual private DistributionsTunableAware {
protected:
  void
  insertTunableDistribution(const FactorExponentialPtr &tunable_distribution);

  TunableClusters::const_iterator insertTunableDistribution(
      const FactorExponentialPtr &tunable_distribution,
      const categoric::VariablesSet &group_sharing_weigth);

  void copyTunableDistribution(
      const distribution::FactorExponential &tunable_distribution);

  TunableClusters::const_iterator copyTunableDistribution(
      const distribution::FactorExponential &tunable_distribution,
      const categoric::VariablesSet &group_sharing_weigth);
};
} // namespace EFG::strct
