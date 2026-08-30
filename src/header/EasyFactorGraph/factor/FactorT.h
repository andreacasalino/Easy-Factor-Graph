/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/Group.h>
#include <EasyFactorGraph/categoric/Range.h>
#include <EasyFactorGraph/misc/Generator.h>
#include <EasyFactorGraph/misc/Intervals.h>

#include <algorithm>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

namespace EFG::factor {
template <std::size_t N>
using SparseDomainPoint = std::pair<categoric::Combination<N>, float>;

template <std::size_t N, typename Transform> class FactorT {
public:
  FactorT(categoric::Combination<N> sizes, misc::Intervals domain);

  static FactorT from_compact_domain(categoric::Combination<N> sizes,
                                     const std::vector<float> &compact_domain);

  template <bool Sort>
  static FactorT
  from_sparse_domain(categoric::Combination<N> sizes,
                     std::vector<SparseDomainPoint<N>> sparse_domain);

  template <typename G>
  requires misc::IsGenerator<G, SparseDomainPoint<N>>
  static FactorT from_sparse_domain_gen(categoric::Combination<N> sizes, G gen);

  template <bool UseTransformed>
  float get(const categoric::Combination<N> &combination) const;

  template <bool UseTransformed, typename Pred>
  void forEachCombination(Pred pred) const;

  template <bool UseTransformed>
  void getValues(std::vector<float> &recipient) const;

  const auto &getVariables() const { return variables_; }

  Transform trsfm;

  void transferIntoPool(misc::MemoryPool &pool) {
    images_.transferIntoPool(pool);
  }

  std::size_t getMemoryFootprint() const {
    return images_.getDelimiters().getMemoryFootprint();
  }

private:
  categoric::Group<N> variables_;
  misc::Intervals images_;
};

template <typename Transform> class FactorT<1, Transform> {
public:
  explicit FactorT(misc::Slot<float> &&images)
      : values{std::forward<misc::Slot<float>>(images)} {}

  explicit FactorT(const std::vector<float> &images)
      : FactorT{misc::Slot<float>::makeOwning(std::span<const float>{images})} {
  }

  float getTrsfm(std::size_t index) const { return trsfm(values.get()[index]); }

  std::span<const float> getAllValues() const { return values.get(); }

  template <bool UseTransformed>
  void getValues(std::vector<float> &recipient) const;

  Transform trsfm;

  std::size_t getVarSize() const { return values.block.size(); }

  void transferIntoPool(misc::MemoryPool &pool) {
    values.transferIntoPool(pool);
  }

  std::size_t getMemoryFootprint() const { return values.getMemoryFootprint(); }

private:
  misc::Slot<float> values;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <std::size_t N, typename Transform>
FactorT<N, Transform>::FactorT(categoric::Combination<N> sizes,
                               misc::Intervals domain)
    : trsfm{}, variables_{sizes}, images_{std::move(domain)} {
  auto delim_block = images_.getDelimiters().get();
  auto it = delim_block.rbegin();
  ++it;
  auto &last_del = *it;

  auto len = variables_.combinationsLen();

  if (last_del.first < len || (last_del.first == len && last_del.second == 0)) {
    return;
  }

  throw Error{"Invalid domain, images are out of range"};
}

template <std::size_t N, typename Transform>
FactorT<N, Transform> FactorT<N, Transform>::from_compact_domain(
    categoric::Combination<N> sizes, const std::vector<float> &compact_domain) {
  categoric::Group<N> vars{sizes};
  if (compact_domain.size() != vars.combinationsLen()) {
    throw Error{"Invalid number of images in compact domain"};
  }
  using Gen = misc::GeneratorMap<misc::Intervals::IntervalPoint, float,
                                 misc::GenFromBlock<float>>;

  misc::GenFromBlock<float> gen{compact_domain};
  Gen gen_map{std::move(gen), [index = std::size_t{0}](float val) mutable {
                return std::make_pair(index++, val);
              }};
  return {sizes, misc::Intervals::from_gen<Gen>(std::move(gen_map))};
}

template <std::size_t N, typename Transform>
template <bool Sort>
FactorT<N, Transform> FactorT<N, Transform>::from_sparse_domain(
    categoric::Combination<N> sizes,
    std::vector<SparseDomainPoint<N>> sparse_domain) {
  categoric::Group<N> vars{sizes};
  // map to combination_index -> image value
  std::vector<misc::Intervals::IntervalPoint> sparse_domain_map;
  sparse_domain_map.reserve(sparse_domain.size());
  for (const auto &[comb, val] : sparse_domain) {
    auto index = vars.combinationIndex(comb);
    sparse_domain_map.emplace_back(std::make_pair(index, val));
  }
  return {sizes,
          misc::Intervals::from_points<Sort>(std::move(sparse_domain_map))};
}

template <std::size_t N, typename Transform> template <typename G>
requires misc::IsGenerator<G, SparseDomainPoint<N>> FactorT<N, Transform>
FactorT<N, Transform>::from_sparse_domain_gen(categoric::Combination<N> sizes,
                                              G gen) {
  categoric::Group<N> vars{sizes};
  using Gen = misc::GeneratorMap<typename misc::Intervals::IntervalPoint,
                                 SparseDomainPoint<N>, G>;
  Gen gen_map{std::move(gen), [&](const SparseDomainPoint<N> &el) {
                auto index = vars.combinationIndex(el.first);
                return std::make_pair(index, el.second);
              }};
  return {sizes, misc::Intervals::from_gen<Gen>(std::move(gen_map))};
}

template <std::size_t N, typename Transform>
template <bool UseTransformed>
float FactorT<N, Transform>::get(
    const categoric::Combination<N> &combination) const {
  auto index = variables_.combinationIndex(combination);
  auto val = images_.get(index);
  if constexpr (UseTransformed) {
    val = trsfm(val);
  }
  return val;
}

template <std::size_t N, typename Transform>
template <bool UseTransformed, typename Pred>
void FactorT<N, Transform>::forEachCombination(Pred pred) const {
  auto images_it = images_.iter();
  categoric::Range<N> comb_it{categoric::Combination<N>{variables_.sizes()}};
  misc::for_each_generated<const categoric::Combination<N> *>(
      std::move(comb_it), [&](const categoric::Combination<N> *comb) {
        auto val = images_it.next();
        if constexpr (UseTransformed) {
          val = trsfm(val);
        }
        pred(std::ref(*comb), val);
      });
}

template <std::size_t N, typename Transform>
template <bool UseTransformed>
void FactorT<N, Transform>::getValues(std::vector<float> &recipient) const {
  auto images_it = images_.iter();
  std::size_t len = variables_.combinationsLen();
  recipient.clear();
  recipient.reserve(len);
  for (std::size_t k = 0; k < len; ++k) {
    auto val = images_it.next();
    if constexpr (UseTransformed) {
      val = trsfm(val);
    }
    recipient.emplace_back(val);
  }
}

template <typename Transform>
template <bool UseTransformed>
void FactorT<1, Transform>::getValues(std::vector<float> &recipient) const {
  recipient.clear();
  auto block = values.get();
  recipient.reserve(block.size());
  for (float val : block) {
    if constexpr (UseTransformed) {
      recipient.push_back(trsfm(val));
    } else {
      recipient.push_back(val);
    }
  }
}
} // namespace EFG::factor
