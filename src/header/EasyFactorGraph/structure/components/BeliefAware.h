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
  std::size_t max_iterations_loopy_propagation;
};

struct ClusterInfo {
  bool tree_or_loopy_graph;
  std::size_t size; // number of involved nodes
};

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
