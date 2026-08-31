/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/factor/FactorT.h>
#include <EasyFactorGraph/factor/SimpleCorrelations.h>
#include <EasyFactorGraph/factor/Transform.h>

namespace EFG::factor {
template <std::size_t N> using Factor = FactorT<N, NullTrasform>;
using UnaryFactor = FactorT<1, NullTrasform>;
using BinaryFactor = FactorT<2, NullTrasform>;

template <std::size_t N>
using FactorExponential = FactorT<N, ExponentialTrasform>;
using UnaryFactorExponential = FactorT<1, ExponentialTrasform>;
using BinaryFactorExponential = FactorT<2, ExponentialTrasform>;

template <std::size_t N, typename Transform = NullTrasform>
FactorT<N, Transform> make_simply_correlated(categoric::VarStateSize var_size) {
  using Gen = SimplyCorrelatedDomainGen<N>;
  return FactorT<N, Transform>::from_sparse_domain_gen(
      categoric::make_same<N>(var_size), Gen{var_size});
}

template <std::size_t N>
FactorExponential<N>
make_exp_simply_correlated(categoric::VarStateSize var_size, float weight) {
  auto res = make_simply_correlated<N, ExponentialTrasform>(var_size);
  res.trsfm.setWeight(weight);
  return res;
}

template <std::size_t N, typename Transform = NullTrasform>
FactorT<N, Transform>
make_simply_anti_correlated(categoric::VarStateSize var_size) {
  using Gen = SimplyAntiCorrelatedDomainGen<N>;
  return {categoric::make_same<N>(var_size),
          misc::Intervals::from_gen(Gen{var_size})};
}

template <std::size_t N>
FactorExponential<N>
make_exp_simply_anti_correlated(categoric::VarStateSize var_size,
                                float weight) {
  auto res = make_simply_anti_correlated<N, ExponentialTrasform>(var_size);
  res.trsfm.setWeight(weight);
  return res;
}

/**
 * @return the probabilities associated to each combination in the domain,
 * when assuming only the existance of this distribution. Such probabilities
 * are actually the normalized images. The order of returned values, refer to
 * the combinations that can be iterated by categoric::GroupRange on the
 * variables representing this distribution.
 */
template <std::size_t N, typename Transform>
void getProbabilities(const FactorT<N, Transform> &fctr,
                      std::vector<float> &recipient);

struct UnaryFactorsMerger {
  UnaryFactorsMerger() = default;

  std::span<float> getMerged() { return {values_}; }

  void reset(categoric::VarStateSize var_size) {
    values_.clear();
    values_.resize(var_size, 1.f);
  }

  template <bool Normalize> void merge(std::span<const float> to_merge) {
    for (std::size_t k = 0; k < values_.size(); ++k) {
      values_[k] *= to_merge[k];
    }
    if constexpr (Normalize) {
      normalize();
    }
  }

  template <bool Normalize, typename Transform>
  void merge_factor(const factor::FactorT<1, Transform> &to_merge) {
    for (std::size_t k = 0; k < values_.size(); ++k) {
      values_[k] *= to_merge.getTrsfm(k);
    }
    if constexpr (Normalize) {
      normalize();
    }
  }

  // mostly for numerical computations purpose
  void normalize();

private:
  // all ones are assumed at the beginning
  std::vector<float> values_;
};

template <typename Transform>
factor::UnaryFactor
make_evidence_message(std::span<float> message_support,
                      const factor::FactorT<2, Transform> &binary_factor,
                      bool evidence_var_is_first,
                      categoric::VarStateSize evidence_value) {
  std::size_t receiver_index = evidence_var_is_first ? 1 : 0;
  std::size_t sender_index = evidence_var_is_first ? 0 : 1;

  binary_factor.template forEachCombination<true>(
      [&](const categoric::Combination<2> &comb, auto val) {
        if (comb[sender_index] == evidence_value) {
          message_support[comb[receiver_index]] = val;
        }
      });

  return {factor::FactorT<1, factor::NullTrasform>{
      misc::Slot<float>::makeNonOwning(message_support)}};
}

factor::UnaryFactor make_all_same(std::span<float> support, float value);

factor::UnaryFactor make_indicator(std::span<float> support,
                                   categoric::VarStateSize value);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <std::size_t N, typename Transform>
void getProbabilities(const FactorT<N, Transform> &fctr,
                      std::vector<float> &recipient) {
  fctr.template getValues<true>(recipient);
  // normalize values
  float sum = 0.f;
  for (const auto &val : recipient) {
    sum += val;
  }
  if (sum == 0.f) {
    float e = 1.f / static_cast<float>(recipient.size());
    for (auto &val : recipient) {
      val = e;
    }
  } else {
    for (auto &val : recipient) {
      val /= sum;
    }
  }
}
} // namespace EFG::factor
