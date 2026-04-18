#include <EasyFactorGraph/misc/UniformSampler.h>

#include <BruteForceQuery.h>
#include <Utils.h>

#include <algorithm>
#include <ranges>
#include <unordered_map>

namespace EFG::test {
namespace {
std::vector<categoric::VarStateSize>
get_sizes(const structure::Structure &ctxt) {
  auto rng = ctxt.nodes | std::views::transform([](const structure::Node &n) {
               return n.var_size;
             });
  return {rng.begin(), rng.end()};
}

template <typename Pred>
void for_each_comb(categoric::RangeVec hidden_rng, Pred pred) {
  misc::for_each_generated<const std::vector<categoric::VarStateSize> *>(
      std::move(hidden_rng), [&](const auto *comb) { pred(*comb); });
}
} // namespace

BruteForce::BruteForce(const structure::Structure &context)
    : context_{context}, comb_rng_{get_sizes(context)} {}

float BruteForce::getZ() const {
  if (!Z_proxy_.has_value()) {
    float Z{0.f};
    for_each_comb(comb_rng_, [&](const auto &comb) {
      Z += getEnergy<false>(
          std::span<const categoric::VarStateSize>{comb.begin(), comb.end()});
    });
    Z_proxy_.emplace(Z);
  }
  return *Z_proxy_;
}

std::vector<float> BruteForce::getMarginals(std::size_t var_index) const {
  std::vector<float> res;
  res.resize(context_.nodes[var_index].var_size, 0);
  for_each_comb(comb_rng_,
                [&](const std::vector<categoric::VarStateSize> &comb) {
                  res[comb[var_index]] += getEnergy<false>(comb) / getZ();
                });
  return factor::make_probabilities(std::move(res));
}

std::vector<float> BruteForce::getMarginals(std::size_t var_a_index,
                                            std::size_t var_b_index) const {
  std::vector<float> res;
  categoric::Group<2> g{std::array<categoric::VarStateSize, 2>{
      context_.nodes[var_a_index].var_size,
      context_.nodes[var_b_index].var_size}};
  res.resize(g.combinationsLen(), 0);
  for_each_comb(
      comb_rng_, [&](const std::vector<categoric::VarStateSize> &comb) {
        auto idx = g.combinationIndex({comb[var_a_index], comb[var_b_index]});
        res[idx] += getEnergy<false>(comb) / getZ();
      });
  return factor::make_probabilities(std::move(res));
}

template <bool Logarithm>
float BruteForce::getEnergy(
    std::span<const categoric::VarStateSize> comb) const {
  float res{Logarithm ? 0 : 1.f};
  auto add_ = [&res](float val) {
    if constexpr (Logarithm) {
      res += logf(val);
    } else {
      res *= val;
    }
  };
  for (std::size_t n = 0; n < context_.nodes.size(); ++n) {
    auto comb_val = comb[n];
    if (context_.nodes[n].evidence == structure::Evidence::NOT_AN_EVIDENCE) {
      for (const auto &c : context_.nodes[n].incoming_messages) {
        if (context_.nodes[c.factor_info.sender_index].evidence !=
            structure::Evidence::NOT_AN_EVIDENCE) {
          auto c_message = context_.getMessageValues(c);
          add_(c_message[comb_val]);
        } else if (c.factor_info.receiver_is_first_in_factor) {
          const auto &info =
              context_.binary_factors[c.factor_info.factor_index];
          categoric::Combination<2> comb_arry{comb_val,
                                              comb[c.factor_info.sender_index]};
          add_(std::visit(
              [&](const auto &factor) {
                return factor.template get<true>(comb_arry);
              },
              info.factor));
        }
        structure::use_factor_if(
            context_.nodes[n].unary_factor,
            [&](const auto &factor) { add_(factor.getAllValues()[comb_val]); });
      }
    } else if (context_.nodes[n].evidence != comb_val) {
      return 0;
    }
  }
  return res;
}

BruteForceGradient::BruteForceGradient(model::RandomField &model,
                                       std::size_t samples_len)
    : BruteForce{model.getStructure()}, model_{&model}, samples_{makeSamples(
                                                            samples_len)} {}

BruteForceGradient::BruteForceGradient(model::ConditionalRandomField &model,
                                       std::size_t samples_len)
    : BruteForce{model.getStructure()}, model_{&model}, samples_{makeSamples(
                                                            samples_len)} {}

void BruteForceGradient::setEvidences(
    model::ConditionalRandomField &crf,
    std::span<const categoric::VarStateSize> comb) const {
  evidences_cache_.clear();
  for (auto ev : context_.permanent_evidences) {
    evidences_cache_.push_back(comb[ev]);
  }
  crf.setEvidences(evidences_cache_);
}

std::shared_ptr<const misc::Samples>
BruteForceGradient::makeSamples(std::size_t len) {
  auto samples = std::make_shared<misc::Samples>(context_.nodes.size());

  std::vector<float> prob;
  std::vector<std::size_t> evidences_cache;
  for_each_comb(comb_rng_, [&](const auto &comb) {
    std::visit(
        [&](auto *model) {
          if constexpr (std::is_same_v<decltype(model),
                                       model::ConditionalRandomField *>) {
            setEvidences(*model, std::span<const categoric::VarStateSize>{
                                     comb.begin(), comb.end()});
          }
        },
        model_);
    prob.push_back(getEnergy<false>(comb) / getZ());
  });

  misc::UniformSampler sampler;
  std::vector<std::size_t> counters;
  counters.resize(prob.size(), 0);
  for (std::size_t i = 0; i < len; ++i) {
    auto idx = sampler.sampleFromDiscrete(prob);
    counters[idx] += 1;
  }

  for_each_comb(comb_rng_, [&, idx = 0](const auto &comb) mutable {
    std::size_t times = counters[idx++];
    for (std::size_t t = 0; t < times; ++t) {
      samples->add(comb);
    }
  });

  return samples;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template float
    BruteForce::getEnergy<true>(std::span<const categoric::VarStateSize>) const;
template float BruteForce::getEnergy<false>(
    std::span<const categoric::VarStateSize>) const;
} // namespace EFG::test
