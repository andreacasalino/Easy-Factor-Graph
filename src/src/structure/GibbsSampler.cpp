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
struct Indices {
  std::vector<std::size_t> evidences;
  std::vector<std::size_t> hidden;
};

Indices identify_indices(const Structure &ctxt) {
  Indices res;
  for (std::size_t i = 0; i < ctxt.nodes.size(); ++i) {
    if (ctxt.nodes[i].evidence == Evidence::NOT_AN_EVIDENCE) {
      res.hidden.push_back(i);
    } else {
      res.evidences.push_back(i);
    }
  }
  return res;
}

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
          ? *context.delta_iterations
          : static_cast<std::size_t>(ceil(context.samples_number * 0.1));
  delta_iterations = std::max<std::size_t>(1, delta_iterations);

  std::size_t burn_out = context.transient.has_value() ? *context.transient
                                                       : 10 * delta_iterations;

  return std::make_pair(delta_iterations, burn_out);
}

template <typename V> categoric::VarStateSize extract_value(const V &val) {
  if constexpr (std::is_same_v<V, categoric::VarStateSize>) {
    return val;
  } else if constexpr (std::is_same_v<V,
                                      std::atomic<categoric::VarStateSize>>) {
    return val.load(std::memory_order::relaxed);
  }
}

template <typename V> void set_value(V &val, categoric::VarStateSize giver) {
  if constexpr (std::is_same_v<V, categoric::VarStateSize>) {
    val = giver;
  } else if constexpr (std::is_same_v<V,
                                      std::atomic<categoric::VarStateSize>>) {
    val.store(giver, std::memory_order::relaxed);
  }
}

template <typename V>
void samplingIteration(std::span<const std::size_t> vars, std::span<V> buffer,
                       SamplingContext &ctxt) {
  auto &model = *ctxt.model;
  for (auto abs_idx : vars) {
    auto &node = model.nodes[abs_idx];
    auto &msg_buffer = ctxt.message_buffer.getBuffer<0>();
    msg_buffer.resize(node.var_size, 0);
    auto &msg_prob_buffer = ctxt.message_buffer.getBuffer<1>();
    msg_prob_buffer.resize(node.var_size, 0);
    ctxt.merger.reset(node.var_size);
    for (auto &conn : node.incoming_messages) {
      if (model.nodes[conn.factor_info.sender_index].evidence ==
          Evidence::NOT_AN_EVIDENCE) {
        categoric::VarStateSize val =
            extract_value(buffer[conn.factor_info.sender_index]);

        std::visit(
            [&](const auto &factor) {
              make_evidence_message(
                  {msg_buffer.begin(), msg_buffer.end()}, factor,
                  !conn.factor_info.receiver_is_first_in_factor, val);
            },
            model.binary_factors[conn.factor_info.factor_index].factor);
        ctxt.merger.template merge<true>(msg_buffer);
      } else {
        ctxt.merger.template merge<true>(model.getMessageValues(conn));
      }
    }
    use_factor_if(node.unary_factor, [&](const auto &factor) {
      ctxt.merger.template mergeFactor<true>(factor);
    });

    factor::UnaryFactor merged_factor{
        misc::Slot<float>::makeNonOwning(ctxt.merger.getMerged())};
    factor::get_probabilities(merged_factor, msg_prob_buffer);

    set_value(buffer[abs_idx],
              ctxt.sampler.sampleFromDiscrete(msg_prob_buffer));
  }
}

template <typename Pred>
void for_each_evidence(const Structure &ctxt, Pred pred) {
  for (std::size_t i = 0; i < ctxt.nodes.size(); ++i) {
    if (ctxt.nodes[i].evidence != Evidence::NOT_AN_EVIDENCE) {
      pred(i, ctxt.nodes[i].evidence);
    }
  }
}

struct SerialDriver {
  SerialDriver(Indices indices, StructurePtr model,
               std::optional<std::size_t> seed);

  void advance(std::vector<categoric::VarStateSize> &result_buffer);

private:
  Indices indices_;
  SamplingContext ctxt_;
};

SerialDriver::SerialDriver(Indices indices, StructurePtr model,
                           std::optional<std::size_t> seed)
    : indices_{std::move(indices)} {
  ctxt_.model = model;
  if (seed.has_value()) {
    ctxt_.sampler.resetSeed(seed.value());
  }
}

void SerialDriver::advance(
    std::vector<categoric::VarStateSize> &result_buffer) {
  samplingIteration(indices_.hidden,
                    std::span<categoric::VarStateSize>{result_buffer.begin(),
                                                       result_buffer.end()},
                    ctxt_);
}

struct ConcurrentDriver {
  ConcurrentDriver(Indices indices, StructurePtr model, misc::WorkerPool &pool,
                   std::optional<std::size_t> seed);

  ~ConcurrentDriver() { delete[] scratch_buffer_; }

  void advance(std::vector<categoric::VarStateSize> &result_buffers);

private:
  misc::WorkerPool &pool_;
  Indices indices_;
  std::vector<SamplingContext> ctxt_;
  std::vector<std::pair<std::size_t, std::size_t>> partitions_;
  std::atomic<categoric::VarStateSize> *scratch_buffer_{nullptr};
};

ConcurrentDriver::ConcurrentDriver(Indices indices, StructurePtr model,
                                   misc::WorkerPool &pool,
                                   std::optional<std::size_t> seed)
    : pool_{pool}, indices_{std::move(indices)},
      partitions_{misc::per_threads_indices_subdivision(
          pool_.size(), indices_.hidden.size())} {
  ctxt_.clear();
  ctxt_.resize(pool.size());
  if (seed.has_value()) {
    for (auto &s : ctxt_) {
      s.model = model;
      s.sampler.resetSeed(seed.value());
    }
  }
  scratch_buffer_ =
      new std::atomic<categoric::VarStateSize>[model->nodes.size()];
  for_each_evidence(*model, [&](std::size_t i, categoric::VarStateSize val) {
    scratch_buffer_[i] = val;
  });
}

void ConcurrentDriver::advance(
    std::vector<categoric::VarStateSize> &result_buffer) {
  pool_.compute([&, len = pool_.size()](std::size_t th_id) {
    auto &partition = partitions_[th_id];
    auto &ctxt = ctxt_[th_id];
    std::span<std::atomic<categoric::VarStateSize>> buffer{
        scratch_buffer_, ctxt.model->nodes.size()};
    samplingIteration(
        std::span<const std::size_t>{indices_.hidden.begin() + partition.first,
                                     indices_.hidden.begin() +
                                         partition.second},
        buffer, ctxt);
  });
  for (auto idx : indices_.hidden) {
    result_buffer[idx] = scratch_buffer_[idx];
  }
}

template <typename Driver>
misc::Samples
make_samples(Driver &driver,
             const GibbsSampler::SamplesGenerationContext &context,
             StructurePtr model) {
  misc::Samples samples{model->nodes.size()};

  auto [delta_iterations, burn_out] = parse_context(context);

  std::vector<categoric::VarStateSize> buffer;
  buffer.resize(model->nodes.size(), 0);
  for_each_evidence(*model, [&](std::size_t i, categoric::VarStateSize val) {
    buffer[i] = val;
  });

  // burn out phase
  for (std::size_t i = 0; i < burn_out; ++i) {
    driver.advance(buffer);
  }

  for (std::size_t i = 0; i < context.samples_number; ++i) {
    driver.advance(buffer);
    samples.add(buffer);
    for (std::size_t t = 0; t < delta_iterations; ++t) {
      driver.advance(buffer);
    }
  }

  return samples;
}
} // namespace

misc::Samples
GibbsSampler::makeSamples(const SamplesGenerationContext &context) {
  misc::Samples samples{context_->nodes.size()};

  auto idx = identify_indices(*context_);
  if (idx.hidden.empty()) {
    throw Error{"Empty hidden set"};
  }

  if (auto *pool = listener_.getPool(); pool) {
    ConcurrentDriver driver{std::move(idx), context_, *pool, context.seed};
    return make_samples(driver, context, context_);
  } else {
    SerialDriver driver{std::move(idx), context_, context.seed};
    return make_samples(driver, context, context_);
  }
}
} // namespace EFG::structure
