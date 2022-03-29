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

namespace EFG::strct {
class GibbsSampler : virtual public StateAware,
                     virtual public BeliefAware,
                     virtual public PoolAware {
public:
  struct SamplesGenerationContext {
    std::size_t samples_number;
    std::optional<std::size_t> delta_iterations;
  };
  /**
   * @brief Use Gibbs sampling to draw samples for the hidden variables,
   * conditioned to the current evidences.
   * @param number of samples to draw
   * @param number of iterations used to evolve the model between the drawing of
   * one sample and another
   */
  std::vector<categoric::Combination>
  getHiddenSetSamples(const SamplesGenerationContext &context,
                      const std::size_t threads = 1) const;
};
} // namespace EFG::strct
