/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/Error.h>
#include <memory>

namespace EFG {
template <typename T> class Cache : std::unique_ptr<T> {
public:
  Cache() = default;

  T &reset(std::unique_ptr<T> new_value = nullptr) {
    auto &ref = cast_();
    ref = std::move(new_value);
    return *ref;
  }

  bool empty() const { return nullptr == cast_(); };

  T *get() {
    throw_if_empty();
    return cast_().get();
  }

  const T *get() const {
    throw_if_empty();
    return cast_().get();
  }

private:
  std::unique_ptr<T> &cast_() {
    return static_cast<std::unique_ptr<T> &>(*this);
  }

  const std::unique_ptr<T> &cast_() const {
    return static_cast<const std::unique_ptr<T> &>(*this);
  }

  void throw_if_empty() const {
    if (nullptr == cast_()) {
      throw Error{"Trying to using empty cache"};
    }
  }
};
} // namespace EFG
