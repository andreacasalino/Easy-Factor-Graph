/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/components/PoolAware.h>
#include <EasyFactorGraph/structure/components/StateAware.h>

#include <memory>

namespace EFG::strct {
enum PropagationKind { SUM, MAP };

struct PropagationContext {
  /**
   * @brief maximum number of iterations to use when trying to calibrate a loopy
   * graph
   */
  std::size_t max_iterations_loopy_propagation;
};

struct ClusterInfo {
  bool tree_or_loopy_graph;
  /**
   * @brief number of nodes that constitutes the graph.
   */
  std::size_t size;
};

/**
 * @brief a structure that can be exposed after having propagated the belief,
 * providing info on the encountered structure.
 */
struct PropagationResult {
  PropagationKind propagation_kind_done;

  bool was_completed;

  std::vector<ClusterInfo> structure_found;
};

class BeliePropagationStrategy {
public:
  virtual ~BeliePropagationStrategy() = default;

  virtual PropagationResult propagateBelief(HiddenClusters &subject,
                                            const PropagationKind &kind,
                                            const PropagationContext &context,
                                            Pool &pool) = 0;
};
using BeliePropagationStrategyPtr = std::unique_ptr<BeliePropagationStrategy>;

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
  PropagationContext &getPropagationContext() { return context; }

  bool hasPropagationResult() const { return nullptr != lastPropagation; }
  const PropagationResult &getLastPropagationResult() const {
    return *this->lastPropagation;
  };

  void setPropagationStrategy(BeliePropagationStrategyPtr strategy);

protected:
  BeliefAware();

  void resetBelief() { lastPropagation.reset(); }
  void propagateBelief(const PropagationKind &kind);
  bool wouldNeedPropagation(const PropagationKind &kind) const;

private:
  PropagationContext context = PropagationContext{1000};

  /**
   * @brief results about the last belief propagation done. It is a
   nullptr until the first propagation is triggered
   */
  std::unique_ptr<PropagationResult> lastPropagation;

  BeliePropagationStrategyPtr propagator;
};
} // namespace EFG::strct
