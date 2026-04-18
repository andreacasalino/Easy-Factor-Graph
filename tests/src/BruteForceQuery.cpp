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
  return make_prob_distr(res);
}

std::vector<float> BruteForce::getMarginals(std::size_t var_a_index,
                                            std::size_t var_b_index) const {
  std::vector<float> res;
  categoric::Group g{std::array<categoric::VarStateSize, 2>{
      context_.nodes[var_a_index].var_size,
      context_.nodes[var_b_index].var_size}};
  res.resize(g.combinationsLen(), 0);
  for_each_comb(
      comb_rng_, [&](const std::vector<categoric::VarStateSize> &comb) {
        auto idx = g.combinationIndex({comb[var_a_index], comb[var_b_index]});
        res[idx] += getEnergy<false>(comb) / getZ();
      });
  return make_prob_distr(res);
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

std::shared_ptr<const misc::Samples>
BruteForceGradient::makeSamples(std::size_t len) {
  auto samples = std::make_shared<misc::Samples>(context_.nodes.size());

  std::vector<float> prob;
  for_each_comb(comb_rng_, [&](const auto &comb) {
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

std::vector<float> BruteForceGradient::getGradientAt() const {
  std::vector<float> res;
  for (const auto &t : context_.tunability.order) {
    res.push_back(std::visit([&](const auto &t) { return getAlfa(t); }, t));
  }

  // TODO distinguish conditional case here !
  for (std::size_t t = 0; t < context_.tunability.order.size(); ++t) {
    res[t] -= std::visit([&](const auto &t) { return getBeta(t); },
                         context_.tunability.order[t]);
  }
  return res;
}

float BruteForceGradient::getAlfa(
    const structure::Tunability::TunableUnaryFactor &t) const {
  float res{0};
  const auto &factor = std::get<factor::UnaryFactorExponential>(
      context_.nodes[t.var_index].unary_factor);
  float samples_size = static_cast<float>(samples_->size());
  misc::for_each_generated<std::span<const categoric::VarStateSize>>(
      samples_->makeIter(), [&](const auto &sample) {
        res += factor.getAllValues()[sample[t.var_index]] / samples_size;
      });
  return res;
}

float BruteForceGradient::getBeta(
    const structure::Tunability::TunableUnaryFactor &t) const {
  float res{0};
  const auto &factor = std::get<factor::UnaryFactorExponential>(
      context_.nodes[t.var_index].unary_factor);
  auto marginals = getMarginals(t.var_index);
  for (categoric::VarStateSize s = 0; s < context_.nodes[t.var_index].var_size;
       ++s) {
    res += factor.getAllValues()[s] * marginals[s];
  }
  return res;
}

float BruteForceGradient::getAlfa(
    const structure::Tunability::TunableBinaryFactor &t) const {
  float res{0};
  float samples_size = static_cast<float>(samples_->size());
  const auto &factor = find_binary_factor(t);
  misc::for_each_generated<std::span<const categoric::VarStateSize>>(
      samples_->makeIter(), [&](const auto &sample) {
        res += factor.get<false>(
                   {sample[t.first_var_index], sample[t.second_var_index]}) /
               samples_size;
      });
  return res;
}

float BruteForceGradient::getBeta(
    const structure::Tunability::TunableBinaryFactor &t) const {
  float res{0};
  const auto &factor = find_binary_factor(t);
  auto marginals = getMarginals(t.first_var_index, t.second_var_index);
  factor.forEachCombination<false>(
      [&, idx = 0](const auto &_, float val) mutable {
        res += val * marginals[idx++];
      });
  return res;
}

float BruteForceGradient::getLogLikelihoodAt() const {
  float Z = 0.f;
  misc::for_each_generated<std::span<const categoric::VarStateSize>>(
      samples_->makeIter(),
      [&](const auto &sample) { Z += getEnergy<true>(sample); });
  float lkl = 0.f;
  float coeff = 1.f / static_cast<float>(samples_->size());
  misc::for_each_generated<std::span<const categoric::VarStateSize>>(
      samples_->makeIter(),
      [&](const auto &sample) { lkl += coeff * getEnergy<true>(sample); });
  return lkl - Z;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template float
    BruteForce::getEnergy<true>(std::span<const categoric::VarStateSize>) const;
template float BruteForce::getEnergy<false>(
    std::span<const categoric::VarStateSize>) const;
} // namespace EFG::test
