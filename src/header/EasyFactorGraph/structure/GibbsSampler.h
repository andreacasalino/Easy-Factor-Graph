/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/bases/BeliefAware.h>
#include <EasyFactorGraph/structure/bases/PoolAware.h>

#include <optional>
#include <random>

namespace EFG::strct {
class UniformSampler {
public:
  UniformSampler();

  std::size_t sampleFromDiscrete(const std::vector<float> &distribution) const;

  void resetSeed(std::size_t newSeed);

private:
  float sample() const { return this->distribution(this->generator); };

  mutable std::default_random_engine generator;
  mutable std::uniform_real_distribution<float> distribution;
};

/**
 * @brief Refer also to https://en.wikipedia.org/wiki/Gibbs_sampling
 */
class GibbsSampler : virtual public StateAware,
                     virtual public BeliefAware,
                     virtual public PoolAware {
public:
  struct SamplesGenerationContext {
    std::size_t samples_number;
    /**
     * @brief number of iterations used to evolve the model between the drawing
     * of one sample and another
     */
    std::optional<std::size_t> delta_iterations;
    /**
     * @brief sets the seed of the random engine.
     * Passing a nullopt will make the sampler to generate a random seed by
     * using the current time.
     */
    std::optional<std::size_t> seed;
    /**
     * @brief number of samples to discard before actually starting the sampling
     * procedure.
     *
     * When nothing is specified, 10 times delta_iterations is assumed.
     */
    std::optional<std::size_t> transient;
  };
  /**
   * @brief Use Gibbs sampling approach to draw empirical samples. Values inside
   * the returned combiantion are ordered with the same order used for the
   * variables returned by getAllVariables().
   *
   * In case some evidences are set, their values will appear as is in the
   * sampled combinations.
   *
   * @param number parameters for the samples generation
   * @param number of threads to use for the samples generation
   */
  std::vector<std::vector<std::size_t>>
  makeSamples(const SamplesGenerationContext &context, std::size_t threads = 1);

  struct SamplerNode {
    std::size_t *value_in_combination;
    const factor::UnaryFactor *static_dependencies;

    struct DynamicDependency {
      categoric::VariablePtr sender;
      const std::size_t *sender_value_in_combination;
      factor::ImmutablePtr factor;
    };
    std::vector<DynamicDependency> dynamic_dependencies;

    bool noChangingDeps(
        const std::unordered_set<const std::size_t *> &will_change) const;
  };

private:
  std::vector<SamplerNode>
  makeSamplerNodes(std::vector<std::size_t> &combination_buffer) const;
};
} // namespace EFG::strct
