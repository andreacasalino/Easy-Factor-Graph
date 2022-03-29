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
   P(var|model,
   * observations), conditioned to the last set evidences.
   * @throw when the passed variable name is not found
   */
  std::vector<float> getMarginalDistribution(const std::string &var,
                                             const std::size_t threads = 1);

  /**
   * @return a factor representing the joint distribution of the subgraph
   * described by the passed variables.
   * @throw when some of the passed variable names are not found
   */
  distribution::Factor
  getJointMarginalDistribution(const std::unordered_set<std::string> &subgroup,
                               const std::size_t threads = 1);

  /**
   * @return the Maximum a Posteriori estimation of a specific variable in
   the
   * model, conditioned to the last set evidences.
   * @throw when the passed variable name is not found
   */
  std::size_t getMAP(const std::string &var, const std::size_t threads = 1);

  /**
   * @return the Maximum a Posteriori estimation of the hidden variables,
   * conditioned to the last set evidences. values are ordered in the same
   way
   * the variables in the hiddden set can be ordered (alfabetic order)
   */
  std::vector<size_t> getHiddenSetMAP(const std::size_t threads = 1);
};
} // namespace EFG::strct
