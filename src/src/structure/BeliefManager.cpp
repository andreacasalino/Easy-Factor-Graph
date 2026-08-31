/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/misc/VectorCache.h>
#include <EasyFactorGraph/structure/BeliefManager.h>

#include <unordered_set>

namespace EFG::structure {
float MessagePassingHandler::eval_diff(std::span<float> pre,
                                       std::span<float> post) {
  auto &prob_pre = cache_.template get_buffer<1>();
  factor::getProbabilities(
      factor::UnaryFactor{misc::Slot<float>::makeNonOwning(pre)}, prob_pre);

  auto &prob_post = cache_.template get_buffer<2>();
  factor::getProbabilities(
      factor::UnaryFactor{misc::Slot<float>::makeNonOwning(post)}, prob_post);

  float res = 0;
  for (std::size_t k = 0; k < prob_pre.size(); ++k) {
    res += std::abs(prob_pre[k] - prob_post[k]);
  }
  return res;
}

template <PropagationKind Kind, bool EvalDiff>
float MessagePassingHandler::compute_message(Structure &context,
                                             MessageMetaData &msg,
                                             std::span<float> recipient) {
  this->UnaryFactorsMerger::reset(
      context.nodes[msg.factor_info.sender_index].var_size);

  for (const auto &el : context.get_message_dependencies_rng(msg)) {
    this->UnaryFactorsMerger::template merge<true>(
        context.getMessageValues(el));
  }
  use_factor_if(context.nodes[msg.factor_info.sender_index].unary_factor,
                [&](const auto &factor) {
                  this->UnaryFactorsMerger::template merge_factor<true>(factor);
                });

  if constexpr (EvalDiff) {
    auto &prev_distr_snap = this->cache_.template get_buffer<0>();
    auto values = context.getMessageValues(msg);
    prev_distr_snap.assign(values.begin(), values.end());
  }

  std::visit(
      [&](const auto &factor) {
        make_passed_message<Kind>(
            recipient, this->UnaryFactorsMerger::getMerged(),
            msg.factor_info.receiver_is_first_in_factor, factor);
        msg.is_available = true;
      },
      context.binary_factors[msg.factor_info.factor_index].factor);

  if constexpr (EvalDiff) {
    return eval_diff(this->cache_.access_buffer<0>(), recipient);
  } else {
    return std::numeric_limits<float>::max();
  }
}

template <PropagationKind Kind> void BeliefManager::propagateBelief() {
  for (auto &cluster : context_->clusters) {
    if (cluster.last_performed_propagation.has_value() &&
        cluster.last_performed_propagation->propagation_kind_done == Kind) {
      continue;
    }
    std::visit(
        [&](auto &propagator) {
          propagator.template propagate<Kind>(*context_, cluster);
        },
        propagator_);
  }
}

namespace {
bool can_compute_message(MessagePassingHandler &mp, Structure &context,
                         MessageMetaData::MessageIdx msg_idx) {
  const auto &msg = context.getMessageData(msg_idx);
  auto rng = context.get_message_dependencies_rng(msg);
  return std::all_of(rng.begin(), rng.end(),
                     [](auto el) { return el.is_available; });
}

template <PropagationKind Kind>
void first_propagation(MessagePassingHandler &mp, Structure &context,
                       HiddenCluster &cluster) {
  auto &order = cluster.order.emplace();
  std::vector<MessageMetaData::MessageIdx> open, open_swap, to_close;
  for (auto var : cluster.variables) {
    for (auto &conn : context.get_active_incoming(var)) {
      conn.is_available = false;
      open.push_back(conn.idx);
    }
  }
  while (!open.empty()) {
    open_swap.clear();
    to_close.clear();
    for (auto conn : open) {
      if (can_compute_message(mp, context, conn)) {
        to_close.push_back(conn);
      } else {
        open_swap.push_back(conn);
      }
    }

    for (auto conn : to_close) {
      mp.template compute_message<Kind, false>(context,
                                               context.getMessageData(conn));
    }

    std::swap(open, open_swap);

    if (to_close.size() == 0) {
      // the rest need calibration
      break;
    }

    order.deterministic_part.add(to_close.begin(), to_close.end());
  }
  order.to_calibrate_part = std::move(open);
}
} // namespace

template <PropagationKind Kind>
void BeliefManager::SerialBeliefPropagator::propagate(Structure &ctxt,
                                                      HiddenCluster &cluster) {
  auto &res = cluster.last_performed_propagation.emplace();
  res.propagation_kind_done = Kind;

  if (cluster.order.has_value()) {
    cluster.order->deterministic_part.for_each(
        [&](MessageMetaData::MessageIdx idx) {
          msg_passing_handler_.compute_message<Kind, false>(
              ctxt, ctxt.getMessageData(idx));
        });
  } else {
    first_propagation<Kind>(msg_passing_handler_, ctxt, cluster);
  }

  if (cluster.order->to_calibrate_part.empty()) {
    res.was_completed = true;
    res.iterations = 1;
  } else {
    auto [was_completed, iterations] =
        calibration_<Kind>(ctxt, cluster.order->to_calibrate_part);
    res.was_completed = was_completed;
    res.iterations = iterations;
  }
}

template <PropagationKind Kind>
std::pair<bool, std::size_t>
BeliefManager::SerialBeliefPropagator::calibration_(
    Structure &ctxt,
    const std::vector<MessageMetaData::MessageIdx> &to_calibrate_part) {
  std::size_t max_iter = ctxt.config.max_calibration_iter;
  float toll = ctxt.config.calibration_tolerance;
  if (calibration_buffer_.empty()) {
    calibration_buffer_ = ctxt.make_messages_pool_swap();
  }

  for (auto conn_idx : to_calibrate_part) {
    auto &conn = ctxt.getMessageData(conn_idx);
    auto conn_message = ctxt.getMessageValues(conn);
    factor::make_all_same(conn_message, 1.f);
    conn.is_available = true;
  }

  for (std::size_t i = 0; i < max_iter; ++i) {
    float max_delta{0};
    for (auto conn_idx : to_calibrate_part) {
      auto &conn = ctxt.getMessageData(conn_idx);
      auto recipient = ctxt.getMessageValues(conn, calibration_buffer_);
      float message_diff = msg_passing_handler_.compute_message<Kind, true>(
          ctxt, conn, recipient);
      max_delta = std::max<float>(max_delta, message_diff);
    }
    if (max_delta < toll) {
      return std::make_pair(true, i + 1);
    }
    ctxt.swap_messages_pool(calibration_buffer_);
  }

  return std::make_pair(false, max_iter);
}

BeliefManager::ConcurrentBeliefPropagator::ConcurrentBeliefPropagator(
    misc::WorkerPool &workers)
    : workers_{workers} {
  msg_passing_handlers_.clear();
  msg_passing_handlers_.resize(workers.size());
}

template <PropagationKind Kind>
void BeliefManager::ConcurrentBeliefPropagator::propagate(
    Structure &ctxt, HiddenCluster &cluster) {
  auto &res = cluster.last_performed_propagation.emplace();
  res.propagation_kind_done = Kind;

  if (cluster.order.has_value()) {
    cluster.order->deterministic_part.for_each_level([&](auto level) {
      workers_.compute([&, size = workers_.size()](std::size_t id) {
        auto &msg_computation = msg_passing_handlers_[id];
        for (std::size_t i = id; i < level.size(); i += size) {
          msg_computation.template compute_message<Kind, false>(
              ctxt, ctxt.getMessageData(level[i]));
        }
      });
    });
  } else {
    first_propagation<Kind>(msg_passing_handlers_.front(), ctxt, cluster);
  }

  if (cluster.order->to_calibrate_part.empty()) {
    res.was_completed = true;
    res.iterations = 1;
  } else {
    auto [was_completed, iterations] =
        calibration_<Kind>(ctxt, cluster.order->to_calibrate_part);
    res.was_completed = was_completed;
    res.iterations = iterations;
  }
}

template <PropagationKind Kind>
std::pair<bool, float> BeliefManager::ConcurrentBeliefPropagator::calibration_(
    Structure &ctxt,
    const std::vector<MessageMetaData::MessageIdx> &to_calibrate_part) {
  std::size_t max_iter = ctxt.config.max_calibration_iter;
  float toll = ctxt.config.calibration_tolerance;
  if (calibration_buffer_.empty()) {
    calibration_buffer_ = ctxt.make_messages_pool_swap();
  }
  std::size_t pool_size = workers_.size();

  // TODO avoid this code repetition
  for (auto idx : to_calibrate_part) {
    auto &conn = ctxt.getMessageData(idx);
    auto conn_message = ctxt.getMessageValues(conn);
    factor::make_all_same(conn_message, 1.f);
    conn.is_available = true;
  }

  auto &diffs = diffs_.get_buffer();
  diffs.resize(pool_size);

  for (std::size_t i = 0; i < max_iter; ++i) {
    workers_.compute([&, size = workers_.size()](std::size_t id) {
      auto &msg_computation = msg_passing_handlers_[id];
      auto &res = diffs[id].value;
      res = 0;
      for (std::size_t i = id; i < to_calibrate_part.size(); i += size) {
        auto &conn = ctxt.getMessageData(to_calibrate_part[i]);
        auto recipient = ctxt.getMessageValues(conn, calibration_buffer_);
        float message_diff =
            msg_computation.compute_message<Kind, true>(ctxt, conn, recipient);
        res = std::max<float>(res, message_diff);
      }
    });
    if (std::max_element(diffs.begin(), diffs.end())->value < toll) {
      return std::make_pair(true, i + 1);
    }
    ctxt.swap_messages_pool(calibration_buffer_);
  }

  return std::make_pair(false, max_iter);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template float
MessagePassingHandler::compute_message<PropagationKind::SUM, true>(
    Structure &, MessageMetaData &);
template float
MessagePassingHandler::compute_message<PropagationKind::SUM, false>(
    Structure &, MessageMetaData &);
template float
MessagePassingHandler::compute_message<PropagationKind::MAP, true>(
    Structure &, MessageMetaData &);
template float
MessagePassingHandler::compute_message<PropagationKind::MAP, false>(
    Structure &, MessageMetaData &);

template void BeliefManager::propagateBelief<PropagationKind::SUM>();
template void BeliefManager::propagateBelief<PropagationKind::MAP>();

template void
BeliefManager::SerialBeliefPropagator::propagate<PropagationKind::SUM>(
    Structure &ctxt, HiddenCluster &);
template void
BeliefManager::SerialBeliefPropagator::propagate<PropagationKind::MAP>(
    Structure &ctxt, HiddenCluster &);

template std::pair<bool, std::size_t>
BeliefManager::SerialBeliefPropagator::calibration_<PropagationKind::SUM>(
    Structure &ctxt, const std::vector<MessageMetaData::MessageIdx> &);
template std::pair<bool, std::size_t>
BeliefManager::SerialBeliefPropagator::calibration_<PropagationKind::MAP>(
    Structure &ctxt, const std::vector<MessageMetaData::MessageIdx> &);

template void
BeliefManager::ConcurrentBeliefPropagator::propagate<PropagationKind::SUM>(
    Structure &ctxt, HiddenCluster &);
template void
BeliefManager::ConcurrentBeliefPropagator::propagate<PropagationKind::MAP>(
    Structure &ctxt, HiddenCluster &);

template std::pair<bool, float>
BeliefManager::ConcurrentBeliefPropagator::calibration_<PropagationKind::SUM>(
    Structure &ctxt, const std::vector<MessageMetaData::MessageIdx> &);
template std::pair<bool, float>
BeliefManager::ConcurrentBeliefPropagator::calibration_<PropagationKind::MAP>(
    Structure &ctxt, const std::vector<MessageMetaData::MessageIdx> &);
} // namespace EFG::structure
