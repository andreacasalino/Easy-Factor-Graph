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

  template <std::size_t Index> auto &get_buffer() {
    auto &res = std::get<Index>(buffer_);
    res.clear();
    return res;
  }

  template <std::size_t Index> auto &access_buffer() {
    return std::get<Index>(buffer_);
  }

private:
  std::tuple<std::vector<Ts>...> buffer_;
};

template <typename T> class VectorCache<T> {
public:
  VectorCache() = default;

  auto &get_buffer() {
    buffer_.clear();
    return buffer_;
  }

  template <std::size_t Index> auto &access_buffer() const { return buffer_; }

private:
  std::vector<T> buffer_;
};
} // namespace EFG::misc
