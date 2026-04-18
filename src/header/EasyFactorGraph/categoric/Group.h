/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/categoric/Types.h>

#include <EasyFactorGraph/Error.h>

#include <array>
#include <vector>

namespace EFG::categoric {
template <typename Support> class GroupT {
public:
  GroupT(Support sizes) : sizes_{sizes}, sizes_prod_{1} {
    for (auto val : sizes) {
      sizes_prod_ *= val;
    }
  }

  const auto &sizes() const { return sizes_; }

  std::size_t combinationsLen() const { return sizes_prod_; }

  std::size_t combinationIndex(const Support &combination) const;

  Support combinationFromIndex(std::size_t idx) const;

  bool check(const Support &combination) const;

private:
  std::size_t sizes_prod_;
  Support sizes_;
};

template <std::size_t N> using Group = GroupT<Combination<N>>;

using GroupVec = GroupT<std::vector<VarStateSize>>;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename Support>
std::size_t
GroupT<Support>::combinationIndex(const Support &combination) const {
  if constexpr (std::is_same_v<Support, std::vector<VarStateSize>>) {
    if (combination.size() != sizes_.size()) {
      throw Error{"Invalid combination size"};
    }
  }
  auto prod = sizes_prod_;
  std::size_t res = 0;
  for (std::size_t k = 0; k < sizes_.size(); ++k) {
    prod /= sizes_[k];
    res += combination[k] * prod;
  }
  return res;
}

template <typename Support>
Support GroupT<Support>::combinationFromIndex(std::size_t idx) const {
  if (sizes_prod_ <= idx) {
    throw Error{"Out of range index for Group"};
  }
  std::size_t prod = sizes_prod_;
  Support res;
  if constexpr (std::is_same_v<Support, std::vector<VarStateSize>>) {
    res.resize(sizes_.size(), 0);
  }
  for (std::size_t i = 0; i < sizes_.size(); ++i) {
    prod /= sizes_[i];
    res[i] = (idx / prod) % sizes_[i];
  }
  return res;
}

template <typename Support>
bool GroupT<Support>::check(const Support &combination) const {
  if constexpr (std::is_same_v<Support, std::vector<VarStateSize>>) {
    if (combination.size() != sizes_.size()) {
      throw Error{"Invalid combination size"};
    }
  }
  for (std::size_t k = 0; k < sizes_.size(); ++k) {
    if (sizes_[k] <= combination[k]) {
      return false;
    }
  }
  return true;
}

template <std::size_t N, typename T> std::array<T, N> make_same(T val) {
  std::array<T, N> res;
  for (auto &res_val : res) {
    res_val = val;
  }
  return res;
}
} // namespace EFG::categoric
