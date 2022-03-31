/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include "SmartPointerUtils.h"

#include <unordered_set>

namespace EFG {
template <typename T>
using SmartSet =
    std::unordered_set<std::shared_ptr<T>, Hasher<T>, Comparator<T>>;

template <typename T, typename Predicate>
void for_each(const SmartSet<T> &subject, const Predicate &pred) {
  for (const auto &element : subject) {
    pred(*element);
  }
};
} // namespace EFG
