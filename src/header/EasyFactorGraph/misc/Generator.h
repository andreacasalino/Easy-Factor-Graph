/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <functional>
#include <optional>
#include <span>
#include <vector>

namespace EFG::misc {
template <typename GenT, typename T>
concept IsGenerator = requires(GenT g) {
  { g.next() } -> std::convertible_to<std::optional<T>>;
};

template <typename T, typename GenT, typename Pred>
requires IsGenerator<GenT, T>
void for_each_generated(GenT gen, Pred pred) {
  for (auto next = gen.next(); next.has_value(); next = gen.next()) {
    auto &next_val = next.value();
    pred(next_val);
  }
}

template <typename T, typename GenT>
requires IsGenerator<GenT, T> std::vector<T> materialize(GenT gen) {
  std::vector<T> res;
  for_each_generated<T>(gen,
                        [&res](T &val) { res.emplace_back(std::move(val)); });
  return res;
}

template <typename U, typename T, typename GenT>
requires IsGenerator<GenT, T>
struct GeneratorMap {
  template <typename Pred>
  GeneratorMap(GenT &&inner, Pred &&pred)
      : inner_{std::forward<GenT>(inner)}, map_{std::forward<Pred>(pred)} {}

  std::optional<U> next() {
    if (auto maybe_val = inner_.next(); maybe_val.has_value()) {
      return std::make_optional(map_(std::move(maybe_val.value())));
    } else {
      return std::nullopt;
    }
  }

private:
  GenT inner_;
  std::function<U(T &&)> map_;
};

// would like to use std::views::iota but got some compile issue on gcc 11
struct Range {
  Range(std::size_t len) : rest_{len} {}

  std::optional<std::size_t> next() {
    if (rest_ == 0) {
      return std::nullopt;
    }
    rest_ -= 1;
    return rest_;
  }

private:
  std::size_t rest_;
};

template <typename T> class GenFromBlock {
public:
  explicit GenFromBlock(std::span<T> src)
      : GenFromBlock{std::span<const T>{src}} {}

  explicit GenFromBlock(std::span<const T> src)
      : owning_block_{}, remainig_block_view_{src} {}

  explicit GenFromBlock(const std::vector<T> &src) 
      : GenFromBlock{std::span<const T>{src.begin(), src.end()}} {}

  explicit GenFromBlock(std::vector<T> &&src)
      : owning_block_{std::forward<std::vector<T>>(src)}, remainig_block_view_{
                                                              owning_block_} {}

  std::optional<T> next() {
    if (remainig_block_view_.empty()) {
      return std::nullopt;
    } else {
      T res = remainig_block_view_[0];
      remainig_block_view_ = {remainig_block_view_.begin() + 1,
                              remainig_block_view_.end()};
      return res;
    }
  }

private:
  // set only when the collection is actually transferred into the generator
  std::vector<T> owning_block_;

  std::span<const T> remainig_block_view_;
};
} // namespace EFG::misc
