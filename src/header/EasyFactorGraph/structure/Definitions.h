/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/misc/ConcurrentSafeLevels.h>

#include <list>
#include <optional>
#include <ranges>
#include <span>

namespace EFG::structure {
enum class PropagationKind { SUM, MAP };

template <std::size_t N>
using FactorOrFactorExponential =
    std::variant<factor::FactorT<N, factor::NullTrasform>,
                 factor::FactorT<N, factor::ExponentialTrasform>>;

template <std::size_t N>
using EmptyOrFactorOrFactorExponential =
    std::variant<std::monostate, factor::FactorT<N, factor::NullTrasform>,
                 factor::FactorT<N, factor::ExponentialTrasform>>;

template <std::size_t N, typename Pred>
void use_factor_if(const EmptyOrFactorOrFactorExponential<N> &factor,
                   Pred pred);

struct MessageMetaData {
  using MessageIdx = std::size_t;

  bool is_available{false};

  // position in the pool of messages
  MessageIdx idx;
  // position of the twin message, i.e. those going
  // receiver_index->sender_index, in the pool of messages
  MessageIdx twin_idx;

  struct FactorInfo {
    bool receiver_is_first_in_factor;
    std::size_t receiver_index;
    std::size_t sender_index;
    std::size_t factor_index;
  };

  FactorInfo factor_info;
};

using PropagationLevels =
    misc::ConcurrentSafeLevels<MessageMetaData::MessageIdx>;

struct PropagationOrder {
  // Theese can be computed once and for all without calibration.
  // Notice that politrees will be made only of such kind of connections,
  // while loopy structures may still have a subset of messages
  // that can be built exactly, with only the rest needing calibration
  PropagationLevels deterministic_part;

  // Theese needs calibration (i.e. existance of loops)
  std::vector<MessageMetaData::MessageIdx> to_calibrate_part;
};

struct PropagationResult {
  PropagationKind propagation_kind_done;
  bool was_completed;
  std::size_t iterations;
};

struct HiddenCluster {
  std::vector<std::size_t> variables;

  // proxies
  std::optional<PropagationOrder> order;
  std::optional<PropagationResult> last_performed_propagation;
};

using HiddenClusters = std::list<HiddenCluster>;

struct Node {
  categoric::VarStateSize var_size;
  categoric::VarStateSize evidence;

  EmptyOrFactorOrFactorExponential<1> unary_factor;

  std::span<MessageMetaData> incoming_messages;

  // intrusive iterator pointing to the cluster this node pertain to (if any
  // otherwise is and end() iterator)
  typename HiddenClusters::iterator hidden_cluster_it;
};

struct Config {
  // when set to 0 the number of cores on the hosting machine is used ... TODO
  std::size_t workers_pool_size{0};
  std::size_t max_calibration_iter = 1000;
  float calibration_tolerance = 1e-5;
};

struct Evidence {
  static const inline categoric::VarStateSize NOT_AN_EVIDENCE =
      std::numeric_limits<categoric::VarStateSize>::max();

  std::size_t var_index;
  categoric::VarStateSize value;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <std::size_t N, typename Pred>
void use_factor_if(const EmptyOrFactorOrFactorExponential<N> &factor,
                   Pred pred) {
  std::visit(
      [&](const auto &factor) {
        if constexpr (!std::is_same_v<decltype(factor),
                                      const std::monostate &>) {
          pred(factor);
        }
      },
      factor);
}
} // namespace EFG::structure
