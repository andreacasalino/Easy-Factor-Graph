/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/StructureAware.h>

namespace EFG::strct {
class StructureTunableAware : public StructureAware {
public:
  /**
   * @brief the clusters of tunable exponential factors.
   * Each element in the returned vector, is a cluster of exponential
   factors sharing the same weight value.
   */
  std::vector<std::vector<
      std::shared_ptr<distribution::factor::modif::FactorExponential>>>
  getFactorsExp() const;

  /**
   * @return the weights of the tunable clusters. For each cluster only
   1 value is returned, since it is shared among the elements in the
   same cluster.
   */
  std::vector<float> getWeights() const;

protected:
  std::size_t numberOfClusters = 0;
  /**
   * @return collection of tunable exponential factors. Each element is
   <exp factor, cluster id>
   */
  std::map<std::shared_ptr<distribution::factor::modif::FactorExponential>,
           std::size_t>
      factorsExp;

private:
  float findWeight(std::size_t cluster) const;
};
} // namespace EFG::strct
