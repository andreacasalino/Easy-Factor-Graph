/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/misc/ConcurrentSafeLevels.h>
#include <EasyFactorGraph/misc/Samples.h>
#include <EasyFactorGraph/misc/UniformSampler.h>
#include <EasyFactorGraph/misc/VectorCache.h>
#include <EasyFactorGraph/structure/Structure.h>
#include <EasyFactorGraph/structure/WorkerPoolManager.h>

#include <deque>
#include <optional>
#include <unordered_map>

namespace EFG::structure {
/**
 * @brief Refer also to https://en.wikipedia.org/wiki/Gibbs_sampling
 */
class GibbsSampler {
public:
  struct SamplesGenerationContext {
    std::size_t samples_number;
    /**
     * @brief number of iterations used to evolve the model between the drawing
     * of one sample and another
     *
     * When nothing is specified, 0.1 * samples_number is assumed
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

    bool include_evidences_in_samples{false};
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
  misc::Samples makeSamples(const SamplesGenerationContext &context);

protected:
  void init(StructurePtr context) {
    context_ = context;
    context_->addListener(listener_);
  }

private:
  StructurePtr context_;
  WorkerPoolActivityListener listener_;
};
} // namespace EFG::structure
