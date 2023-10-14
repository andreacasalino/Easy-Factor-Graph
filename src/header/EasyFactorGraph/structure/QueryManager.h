/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/bases/BeliefAware.h>
#include <EasyFactorGraph/structure/bases/PoolAware.h>

#include <algorithm>

namespace EFG::strct {
class QueryManager : virtual public StateAware,
                     virtual public BeliefAware,
                     virtual public PoolAware {
public:
  /**
   * @return the marginal probabilty of the passed variable, i.e.
   * P(var|observations), conditioned to the last set of evidences.
   * @param the involved variable
   * @param the number of threads to use for propagating the belief before
   * returning the result.
   * @throw when the passed variable name is not found
   */
  std::vector<float> getMarginalDistribution(const categoric::VariablePtr &var,
                                             std::size_t threads = 1) {
    return marginalQuery_<PropagationKind::SUM>(var, threads);
  }

  /**
   * @brief same as getMarginalDistribution(const categoric::VariablePtr &,
   * std::size_t), but passing the name of the variable, which is
   * internally searched.
   */
  std::vector<float> getMarginalDistribution(const std::string &var,
                                             std::size_t threads = 1);

  /**
   * @return a factor representing the joint distribution of the subgraph
   * described by the passed set of variables.
   * @param the involved variables
   * @param the number of threads to use for propagating the belief before
   * returning the result.
   * @throw when some of the passed variable names are not found
   */
  factor::Factor
  getJointMarginalDistribution(const categoric::Group &subgroup,
                               std::size_t threads = 1);

  /**
   * @brief same as getJointMarginalDistribution(const categoric::VariablesSet
   * &, std::size_t), but passing the names of the variables, which are
   * internally searched.
   *
   * @throw in case the passed set of variables is not representative of a valid
   * group
   */
  factor::Factor
  getJointMarginalDistribution(const std::vector<std::string> &subgroup,
                               std::size_t threads = 1);

  /**
   * @return the Maximum a Posteriori estimation of a specific variable in
   * the model, conditioned to the last set of evidences.
   * @param the involved variable
   * @param the number of threads to use for propagating the belief before
   * returning the result.
   * @throw when the passed variable name is not found
   */
  std::size_t getMAP(const categoric::VariablePtr &var,
                     std::size_t threads = 1);

  /**
   * @brief same as getMAP(const categoric::VariablePtr &,
   * std::size_t), but passing the name of the variable, which is
   * internally searched.
   */
  std::size_t getMAP(const std::string &var, std::size_t threads = 1);

  /**
   * @return the Maximum a Posteriori estimation of the hidden variables,
   * conditioned to the last set of evidences. Values are ordered with the same
   * order used by the set of variables returned in getHiddenVariables()
   * @param the number of threads to use for propagating the belief before
   * returning the result.
   */
  std::vector<size_t> getHiddenSetMAP(std::size_t threads = 1);

private:
  static void throwInexistentVar(const std::string &var);

  std::vector<float> getMarginalDistribution(const NodeLocation &location);

  template <PropagationKind Kind> void checkPropagation_(std::size_t threads) {
    if (wouldNeedPropagation(Kind)) {
      ScopedPoolActivator activator(*this, threads);
      propagateBelief(Kind);
    }
  }

  template <PropagationKind Kind>
  std::vector<float> marginalQuery_(const categoric::VariablePtr &var,
                                    std::size_t threads) {
    checkPropagation_<Kind>(threads);
    auto location = locate(var);
    if (!location) {
      throwInexistentVar(var->name());
    }
    return getMarginalDistribution(*location);
  }
};
} // namespace EFG::strct
