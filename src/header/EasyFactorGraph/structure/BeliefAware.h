/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/Node.h>

namespace EFG::strct {
enum PropagationKind { Sum, MAP };

struct PropagationContext {
  std::size_t max_iterations_loopy_propagation;
  std::size_t threads_to_use;
};

struct PropagationResult {
  PropagationKind propagation_kind_done;
  // std::size_t iterationsRequired;
  bool was_completed;
};

class BeliefAware {
public:
  virtual ~BeliefAware() = default;

  const PropagationContext &getPropagationContext() const { return context; }
  void setPropagationContext(const PropagationContext &cntxt) {
    context = cntxt;
  };

  bool hasPropagationResult() const { return nullptr != lastPropagation; }
  const PropagationResult &getLastPropagationResult() const {
    return *this->lastPropagation;
  };

protected:
  BeliefAware() = default;

  void lazyBeliefPropagation(const PropagationKind &kind);
  void resetBelief() { lastPropagation.reset(); }

  virtual PropagationResult propagateBelief(const PropagationKind &kind) = 0;

private:
  PropagationContext context;
  /**
   * @brief results about the last belief propagation done. It is a
   nullptr until the first propagation is triggered
   */
  std::unique_ptr<PropagationResult> lastPropagation;
};
} // namespace EFG::strct
