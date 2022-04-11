/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/components/BeliefAware.h>
#include <EasyFactorGraph/structure/components/PoolAware.h>

namespace EFG::strct {
class QueryManager : virtual public StateAware,
                     virtual public BeliefAware,
                     virtual public PoolAware {
public:
  /**
   * @return the marginal probabilty of the passed variable, i.e.
   * P(var|observations), conditioned to the last set of evidences.
   * @param the involved variable name
   * @param the number of threads to use for propagating the belief before
   * returning the result.
   * @throw when the passed variable name is not found
   */
  std::vector<float> getMarginalDistribution(const std::string &var,
                                             const std::size_t threads = 1);

  /**
   * @return a factor representing the joint distribution of the subgraph
   * described by the passed set of variables.
   * @param the involved variables names
   * @param the number of threads to use for propagating the belief before
   * returning the result.
   * @throw when some of the passed variable names are not found
   */
  std::unique_ptr<distribution::Distribution>
  getJointMarginalDistribution(const std::unordered_set<std::string> &subgroup,
                               const std::size_t threads = 1);

  /**
   * @return the Maximum a Posteriori estimation of a specific variable in
   * the model, conditioned to the last set of evidences.
   * @param the involved variable name
   * @param the number of threads to use for propagating the belief before
   * returning the result.
   * @throw when the passed variable name is not found
   */
  std::size_t getMAP(const std::string &var, const std::size_t threads = 1);

  /**
   * @return the Maximum a Posteriori estimation of the hidden variables,
   * conditioned to the last set of evidences. Values are ordered with the same
   * order used by the set of variables returned in getHiddenVariables()
   * @param the number of threads to use for propagating the belief before
   * returning the result.
   */
  std::vector<size_t> getHiddenSetMAP(const std::size_t threads = 1);
};
} // namespace EFG::strct
