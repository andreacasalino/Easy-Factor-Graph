/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/misc/Generator.h>
#include <EasyFactorGraph/misc/VectorCache.h>
#include <EasyFactorGraph/structure/Definitions.h>
#include <EasyFactorGraph/structure/ModelSeed.h>
#include <EasyFactorGraph/structure/Structure.h>
#include <EasyFactorGraph/structure/WorkerPoolManager.h>

#include <memory>
#include <ranges>
#include <span>
#include <stack>
#include <thread>
#include <unordered_map>
#include <variant>

namespace EFG::structure {
template <PropagationKind Kind, typename TransformT>
factor::UnaryFactor
make_passed_message(std::span<float> message_support,
                    std::span<const float> sender_merged_unaries,
                    bool receiver_is_first,
                    const factor::FactorT<2, TransformT> &binary_factor) {
  std::size_t receiver_index = receiver_is_first ? 0 : 1;
  std::size_t receiver_var_size =
      binary_factor.getVariables().sizes()[receiver_index];
  std::size_t sender_index = receiver_is_first ? 1 : 0;

  float *reduced_values = message_support.data();

  if constexpr (Kind == PropagationKind::SUM) {
    std::fill_n(reduced_values, receiver_var_size, 0);
  } else if constexpr (Kind == PropagationKind::MAP) {
    std::fill_n(reduced_values, receiver_var_size,
                std::numeric_limits<float>::min());
  }

  binary_factor.template forEachCombination<true>(
      [&](const categoric::Combination<2> &comb, auto val) {
        val *= sender_merged_unaries[comb[sender_index]];
        if constexpr (Kind == PropagationKind::SUM) {
          reduced_values[comb[receiver_index]] += val;
        } else if constexpr (Kind == PropagationKind::MAP) {
          reduced_values[comb[receiver_index]] =
              std::max<float>(reduced_values[comb[receiver_index]], val);
        }
      });

  return {factor::FactorT<1, factor::NullTrasform>{
      misc::Slot<float>::makeNonOwning(message_support)}};
}

// TODO shoulnd't this too occupy an entire cache line to avoid false sharing?
class MessagePassingHandler : factor::UnaryFactorsMerger {
public:
  MessagePassingHandler() = default;

  template <PropagationKind Kind, bool EvalDiff>
  float compute_message(Structure &context, MessageMetaData &msg) {
    auto msg_value = context.getMessageValues(msg);
    return compute_message<Kind, EvalDiff>(context, msg, msg_value);
  }

  template <PropagationKind Kind, bool EvalDiff>
  float compute_message(Structure &context, MessageMetaData &msg,
                        std::span<float> recipient);

private:
  float eval_diff(std::span<float> pre, std::span<float> post);

  misc::VectorCache<float, float, float> cache_;
};

class BeliefManager {
  struct SerialBeliefPropagator {
    SerialBeliefPropagator() = default;

    template <PropagationKind Kind>
    void propagate(Structure &ctxt, HiddenCluster &cluster);

  private:
    template <PropagationKind Kind>
    std::pair<bool, std::size_t> calibration_(
        Structure &ctxt,
        const std::vector<MessageMetaData::MessageIdx> &to_calibrate_part);

    MessagePassingHandler msg_passing_handler_;
    std::vector<float> calibration_buffer_;
  };

  struct ConcurrentBeliefPropagator {
    ConcurrentBeliefPropagator(misc::WorkerPool &workers);

    template <PropagationKind Kind>
    void propagate(Structure &ctxt, HiddenCluster &cluster);

  private:
    template <PropagationKind Kind>
    std::pair<bool, float> calibration_(
        Structure &ctxt,
        const std::vector<MessageMetaData::MessageIdx> &to_calibrate_part);

    // aligned as an entire cache line to avoid false sharing
    struct alignas(64) Value {
      float value{0};

      bool operator<(const Value &o) const { return value < o.value; }
    };
    misc::VectorCache<Value> diffs_;

    std::vector<MessagePassingHandler> msg_passing_handlers_;
    std::vector<float> calibration_buffer_;
    misc::WorkerPool &workers_;
  };

public:
  BeliefManager() : listener_{*this} {}

  template <PropagationKind Kind> void propagateBelief();

protected:
  void init(StructurePtr context) {
    context_ = context;
    context_->registerManager(*this);
    context_->addListener(listener_);
  }

private:
  struct Listener : WorkerPoolActivityListener {
    Listener(BeliefManager &source) : source_{source} {}

    void updatePool(misc::WorkerPool *pool) final {
      if (pool) {
        source_.propagator_.emplace<ConcurrentBeliefPropagator>(*pool);
      } else {
        // serial propagator
        source_.propagator_.emplace<SerialBeliefPropagator>();
      }
    }

  private:
    BeliefManager &source_;
  };

  Listener listener_;
  std::variant<SerialBeliefPropagator, ConcurrentBeliefPropagator> propagator_;
  StructurePtr context_;
};
} // namespace EFG::structure
