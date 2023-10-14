/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/bases/PoolAware.h>
#include <EasyFactorGraph/structure/bases/StateAware.h>

#include <optional>

namespace EFG::strct {

class LoopyBeliefPropagationStrategy {
public:
  virtual ~LoopyBeliefPropagationStrategy() = default;

  virtual bool propagateBelief(HiddenCluster &subject, PropagationKind kind,
                               const PropagationContext &context,
                               Pool &pool) = 0;
};
using LoopyBeliefPropagationStrategyPtr =
    std::unique_ptr<LoopyBeliefPropagationStrategy>;

/**
 * @brief The propagation relies on a concrete implementation of a
 * BeliePropagationStrategy. In case no other is specified, a default one,
 * BaselineBeliefPropagator, is instantiated and used internally. You can
 * override this default propagator using setPropagationStrategy(...).
 */
class BeliefAware : virtual public StateAware, virtual public PoolAware {
public:
  virtual ~BeliefAware() = default;

  const PropagationContext &getPropagationContext() const { return context; }
  void setPropagationContext(const PropagationContext &ctxt) { context = ctxt; }

  bool hasPropagationResult() const { return lastPropagation.has_value(); }
  const PropagationResult &getLastPropagationResult() const {
    return *this->lastPropagation;
  };

  void setLoopyPropagationStrategy(LoopyBeliefPropagationStrategyPtr strategy);

protected:
  BeliefAware();

  void resetBelief() { lastPropagation.reset(); }
  bool wouldNeedPropagation(PropagationKind kind) const;
  void propagateBelief(PropagationKind kind);

private:
  PropagationContext context = PropagationContext{1000};

  /**
   * @brief results about the last belief propagation done. It is a
   nullopt until the first propagation is triggered
   */
  std::optional<PropagationResult> lastPropagation;

  LoopyBeliefPropagationStrategyPtr loopy_propagator;
};
} // namespace EFG::strct
