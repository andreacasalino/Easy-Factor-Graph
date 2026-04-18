/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/misc/MemoryPool.h>
#include <EasyFactorGraph/misc/VectorCache.h>
#include <EasyFactorGraph/structure/BeliefManager.h>

#include <algorithm>
#include <optional>
#include <ranges>
#include <span>
#include <variant>

namespace EFG::structure {
class QueryManager {
public:
  /**
   * @param the marginal probabilty of the passed variable, i.e.
   * P(var|observations), conditioned to the last set of evidences.
   * @param the involved variable
   * @param the number of threads to use for propagating the belief before
   * returning the result.
   * @throw when the passed variable name is not found
   */
  void getMarginalDistribution(std::vector<float> &recipient,
                               std::size_t variable_index) {
    auto distr = getMarginal_<PropagationKind::SUM>(variable_index);
    factor::getProbabilities(
        factor::UnaryFactor{misc::TransferableBlock{distr}}, recipient);
  }

  void getJointMarginalDistributionProbabilities(
      std::vector<float> &recipient, const std::vector<std::size_t> &vars) {
    getJointMarginalDistributionProbabilities_(
        recipient, std::span<const std::size_t>{vars});
  }

  template <typename... ARGS>
  void getJointMarginalDistributionProbabilities(std::vector<float> &recipient,
                                                 std::size_t var_first,
                                                 std::size_t var_second,
                                                 ARGS &&...others) {
    auto &vars = cache_.get_buffer<0>();
    vars.push_back(var_first);
    vars.push_back(var_second);
    (vars.push_back(others), ...);

    getJointMarginalDistributionProbabilities_(
        recipient, std::span<const std::size_t>{vars});
  }

  template <std::size_t N>
  factor::FactorT<N, factor::NullTrasform>
  getJointMarginalDistribution(const std::array<std::size_t, N> &vars) {
    static_assert(1 < N,
                  "Joint distribution shall involve at least 2 variables!");

    auto &buffer = cache_.get_buffer<1>();
    getJointMarginalDistributionProbabilities_(
        buffer, std::span<const std::size_t>{vars.data(), N});

    std::array<categoric::VarStateSize, N> vars_sizes;
    for (std::size_t i = 0; i < vars.size(); ++i) {
      vars_sizes[i] = context_->nodes[i].var_size;
    }

    return factor::FactorT<N, factor::NullTrasform>::from_compact_domain(
        vars_sizes, buffer);
  }

  /**
   * @return the Maximum a Posteriori estimation of a specific variable in
   * the model, conditioned to the last set of evidences.
   * @param the involved variable
   * @param the number of threads to use for propagating the belief before
   * returning the result.
   * @throw when the passed variable name is not found
   */
  categoric::VarStateSize getMAP(std::size_t variable_index) {
    auto distr = getMarginal_<PropagationKind::MAP>(variable_index);
    auto it = std::max_element(distr.begin(), distr.end());
    return static_cast<std::size_t>(it - distr.begin());
  }

  /**
   * @return the Maximum a Posteriori estimation of the hidden variables,
   * conditioned to the last set of evidences. Values are ordered with the
   same
   * order used by the set of variables returned in getHiddenVariables()
   * @param the number of threads to use for propagating the belief before
   * returning the result.
   */
  void getHiddenSetMAP(std::vector<categoric::VarStateSize> &recipient);

  const auto &getStructure() const { return *context_; }

  ~QueryManager();

protected:
  QueryManager();

  void init(StructurePtr context) { context_ = context; }

private:
  template <PropagationKind Kind>
  std::span<float> getMarginal_(std::size_t variable_index) {
    if (context_->nodes.size() <= variable_index) {
      throw Error{
          "Cannot perform marginal query on an index that is out of range"};
    }

    context_->getManager<BeliefManager>()->template propagateBelief<Kind>();
    return gatherNodeUnaries_(variable_index);
  }

  std::span<float> gatherNodeUnaries_(std::size_t variable_index);

  void
  getJointMarginalDistributionProbabilities_(std::vector<float> &recipient,
                                             std::span<const std::size_t> vars);

  void *joint_distribution_builder_{nullptr};

  misc::VectorCache<std::size_t, float> cache_;
  factor::UnaryFactorsMerger merger_;

  StructurePtr context_;
};
} // namespace EFG::structure
