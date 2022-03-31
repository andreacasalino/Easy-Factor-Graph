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
template <typename T> struct Hasher {
  std::size_t operator()(const std::shared_ptr<T> &subject) const {
    if (nullptr == subject) {
      throw Error{"can't hash nullptr"};
    }
    return std::hash<T>{}(*subject);
  }
};

template <typename T> struct Comparator {
  bool operator()(const std::shared_ptr<T> &a,
                  const std::shared_ptr<T> &b) const {
    return *a == *b;
  }
};
} // namespace EFG
