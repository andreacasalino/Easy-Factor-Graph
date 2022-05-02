/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/components/BeliefAware.h>
#include <EasyFactorGraph/structure/components/PoolAware.h>

#include <optional>
#include <random>

namespace EFG::strct {
class UniformSampler {
public:
  UniformSampler();

  std::size_t sampleFromDiscrete(const std::vector<float> &distribution) const;

  void resetSeed(const std::size_t &newSeed);

private:
  inline float sample() const { return this->distribution(this->generator); };

  mutable std::default_random_engine generator;
  mutable std::uniform_real_distribution<float> distribution;
};

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
  std::vector<categoric::Combination>
  makeSamples(const SamplesGenerationContext &context,
              const std::size_t threads = 1);
};
} // namespace EFG::strct
