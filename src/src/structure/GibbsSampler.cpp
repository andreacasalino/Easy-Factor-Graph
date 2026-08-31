/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/structure/GibbsSampler.h>

#include <algorithm>
#include <ranges>

namespace EFG::structure {
namespace {
struct HiddenSet {
  std::vector<std::size_t> var_indices_flat;

  std::unordered_map<std::size_t /* var index */,
                     std::size_t /* position in sample buffer */>
      table;

  template <typename Pred>
  void forEachVar(std::size_t offset, std::size_t stride, Pred pred) const {
    for (std::size_t i = offset; i < var_indices_flat.size(); i += stride) {
      pred(var_indices_flat[i], i);
    }
  }
};

struct alignas(64) SamplingContext {
  StructurePtr model;
  misc::UniformSampler sampler;
  factor::UnaryFactorsMerger merger;
  misc::VectorCache<float, float> message_buffer;
};

std::pair<std::size_t, std::size_t>
parse_context(const GibbsSampler::SamplesGenerationContext &context) {
  std::size_t delta_iterations =
      context.delta_iterations.has_value()
          ? context.delta_iterations.value()
          : static_cast<std::size_t>(ceil(context.samples_number * 0.1));
  delta_iterations = std::max<std::size_t>(1, delta_iterations);

  std::size_t burn_out = context.transient.has_value()
                             ? context.transient.value()
                             : 10 * delta_iterations;

  return std::make_pair(delta_iterations, burn_out);
}

HiddenSet identifyHiddenSetIndices(Structure &context) {
  HiddenSet res;
  std::size_t node_i = 0;
  std::size_t buffer_index = 0;
  for (const auto &node : context.nodes) {
    if (node.evidence == Evidence::NOT_AN_EVIDENCE) {
      res.var_indices_flat.push_back(node_i);
      res.table.emplace(node_i, buffer_index++);
    }
    node_i += 1;
  }
  return res;
}

void samplingIteration(const HiddenSet &hidden_set,
                       std::vector<categoric::VarStateSize> &buffer,
                       std::size_t offset, std::size_t stride,
                       SamplingContext &ctxt) {
  auto &model = *ctxt.model;
  hidden_set.forEachVar(offset, stride, [&](auto var_index, auto buffer_index) {
    auto &node = model.nodes[var_index];
    auto &msg_buffer = ctxt.message_buffer.get_buffer<0>();
    msg_buffer.resize(node.var_size, 0);
    auto &msg_prob_buffer = ctxt.message_buffer.get_buffer<1>();
    msg_prob_buffer.resize(node.var_size, 0);
    ctxt.merger.reset(node.var_size);
    for (auto &conn : node.incoming_messages) {
      if (model.nodes[conn.factor_info.sender_index].evidence ==
          Evidence::NOT_AN_EVIDENCE) {
        auto sender_buffer_index =
            hidden_set.table.find(conn.factor_info.sender_index)->second;
        std::visit(
            [&](const auto &factor) {
              make_evidence_message(
                  {msg_buffer.begin(), msg_buffer.end()}, factor,
                  !conn.factor_info.receiver_is_first_in_factor,
                  buffer[sender_buffer_index]);
              ctxt.merger.template merge<true>(msg_buffer);
            },
            model.binary_factors[conn.factor_info.factor_index].factor);
      } else {
        ctxt.merger.template merge<true>(model.getMessageValues(conn));
      }
    }
    use_factor_if(node.unary_factor, [&](const auto &factor) {
      ctxt.merger.template merge_factor<true>(factor);
    });

    factor::UnaryFactor merged_factor{
        misc::Slot<float>::makeNonOwning(ctxt.merger.getMerged())};
    factor::getProbabilities(merged_factor, msg_prob_buffer);

    buffer[buffer_index] = ctxt.sampler.sampleFromDiscrete(msg_prob_buffer);
  });
}

struct SerialDriver {
  SerialDriver(StructurePtr model, std::optional<std::size_t> seed);

  void advance(const HiddenSet &hidden_set,
               std::vector<categoric::VarStateSize> &buffer);

private:
  SamplingContext ctxt_;
};

SerialDriver::SerialDriver(StructurePtr model,
                           std::optional<std::size_t> seed) {
  ctxt_.model = model;
  if (seed.has_value()) {
    ctxt_.sampler.resetSeed(seed.value());
  }
}

void SerialDriver::advance(const HiddenSet &hidden_set,
                           std::vector<categoric::VarStateSize> &buffer) {
  samplingIteration(hidden_set, buffer, 0, 1, ctxt_);
}

struct ConcurrentDriver {
  ConcurrentDriver(StructurePtr model, misc::WorkerPool &pool,
                   std::optional<std::size_t> seed,
                   const HiddenSet &hidden_set);

  void advance(const HiddenSet &hidden_set,
               std::vector<categoric::VarStateSize> &buffer);

private:
  misc::WorkerPool &pool_;
  std::vector<SamplingContext> ctxt_;
  misc::ConcurrentSafeLevels<std::size_t> var_indices_levels_;
};

ConcurrentDriver::ConcurrentDriver(StructurePtr model, misc::WorkerPool &pool,
                                   std::optional<std::size_t> seed,
                                   const HiddenSet &hidden_set)
    : pool_{pool} {
  ctxt_.clear();
  ctxt_.resize(pool.size());
  if (seed.has_value()) {
    for (auto &s : ctxt_) {
      s.model = model;
      s.sampler.resetSeed(seed.value());
    }
  }

  std::vector<std::size_t> open{hidden_set.var_indices_flat}, open_swap;
  std::unordered_set<std::size_t> this_level;
  while (!open.empty()) {
    open_swap.clear();
    this_level.clear();
    for (auto var_index : open) {
      auto deps = model->nodes[var_index].incoming_messages |
                  std::views::transform([](const MessageMetaData &conn) {
                    return conn.factor_info.sender_index;
                  });
      bool dep_in_this_level =
          std::any_of(deps.begin(), deps.end(), [&](auto dep_idx) {
            return this_level.contains(dep_idx);
          });
      if (dep_in_this_level) {
        open_swap.push_back(var_index);
      } else {
        this_level.emplace(var_index);
      }
    }
    var_indices_levels_.add(this_level.begin(), this_level.end());
    std::swap(open, open_swap);
  }
}

void ConcurrentDriver::advance(const HiddenSet &hidden_set,
                               std::vector<categoric::VarStateSize> &buffer) {
  var_indices_levels_.for_each_level([&](std::span<const std::size_t> level) {
    pool_.compute([&, len = pool_.size()](std::size_t th_id) {
      samplingIteration(hidden_set, buffer, th_id, len, ctxt_[th_id]);
    });
  });
}

template <typename Driver, typename Listener>
void make_samples(Driver &driver,
                  const GibbsSampler::SamplesGenerationContext &context,
                  HiddenSet &hidden_set, StructurePtr model, Listener &lstnr) {
  auto [delta_iterations, burn_out] = parse_context(context);

  std::vector<categoric::VarStateSize> buffer;
  buffer.resize(hidden_set.var_indices_flat.size(), 0);

  // burn out phase
  for (std::size_t i = 0; i < burn_out; ++i) {
    driver.advance(hidden_set, buffer);
  }

  for (std::size_t i = 0; i < context.samples_number; ++i) {
    driver.advance(hidden_set, buffer);
    lstnr.on_sample(
        std::span<categoric::VarStateSize>{buffer.begin(), buffer.end()});
    for (std::size_t t = 0; t < delta_iterations; ++t) {
      driver.advance(hidden_set, buffer);
    }
  }
}

struct SamplesAsIs {
  SamplesAsIs(std::size_t sample_len) : samples{sample_len} {}

  misc::Samples samples;

  void on_sample(std::span<categoric::VarStateSize> val) { samples.add(val); }
};

struct SamplesWithEvidences {
  SamplesWithEvidences(HiddenSet hidden, StructurePtr ctxt)
      : samples{ctxt->nodes.size()}, hidden_{std::move(hidden)}, ctxt_{ctxt} {
    buffer_.resize(ctxt->nodes.size(), 0);
    for (std::size_t n = 0; n < ctxt_->nodes.size(); ++n) {
      if (ctxt_->nodes[n].evidence != Evidence::NOT_AN_EVIDENCE) {
        buffer_[n] = ctxt_->nodes[n].evidence;
      }
    }
  }

  void on_sample(std::span<categoric::VarStateSize> val) {
    for (std::size_t k = 0; k < hidden_.var_indices_flat.size(); ++k) {
      auto idx = hidden_.var_indices_flat[k];
      buffer_[idx] = val[k];
    }
    samples.add(buffer_);
  }

  misc::Samples samples;

private:
  std::vector<categoric::VarStateSize> buffer_;
  HiddenSet hidden_;
  StructurePtr ctxt_;
};
} // namespace

misc::Samples
GibbsSampler::makeSamples(const SamplesGenerationContext &context) {
  auto hidden_set = identifyHiddenSetIndices(*context_);
  if (hidden_set.var_indices_flat.empty()) {
    throw Error{"Empty hidden set"};
  }

  auto compute_ = [&](auto lstnr) {
    if (auto *pool = listener_.getPool(); pool) {
      ConcurrentDriver driver{context_, *pool, context.seed, hidden_set};
      make_samples(driver, context, hidden_set, context_, lstnr);
    } else {
      SerialDriver driver{context_, context.seed};
      make_samples(driver, context, hidden_set, context_, lstnr);
    }

    return std::move(lstnr.samples);
  };

  if (context.include_evidences_in_samples) {
    return compute_(SamplesWithEvidences{hidden_set, context_});
  } else {
    return compute_(SamplesAsIs{hidden_set.var_indices_flat.size()});
  }
}
} // namespace EFG::structure
