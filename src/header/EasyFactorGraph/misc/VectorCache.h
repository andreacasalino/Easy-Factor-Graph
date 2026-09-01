/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <tuple>
#include <vector>

namespace EFG::misc {
template <typename... Ts> class VectorCache {
public:
  VectorCache() = default;

  template <std::size_t Index, bool Clear = true> auto &get_buffer() {
    auto &res = std::get<Index>(buffer_);
    if constexpr (Clear) {
      res.clear();
    }
    return res;
  }

private:
  std::tuple<std::vector<Ts>...> buffer_;
};

template <typename T> class VectorCache<T> {
public:
  VectorCache() = default;

  template <bool Clear = true> auto &get_buffer() {
    if constexpr (Clear) {
      buffer_.clear();
    }
    return buffer_;
  }

private:
  std::vector<T> buffer_;
};
} // namespace EFG::misc
