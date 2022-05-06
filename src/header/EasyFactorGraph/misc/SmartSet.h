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
/**
 * @brief An unordered_set that stores shared pointers that can't be null.
 *
 * Elements are hashed by hashing the elements wrapped inside the shared
 * pointer.
 *
 * Elements are compared by comparing the elements wrapped inside the
 * shared pointer.
 */
template <typename T>
using SmartSet =
    std::unordered_set<std::shared_ptr<T>, Hasher<T>, Comparator<T>>;

template <typename T, typename Predicate>
void for_each(const SmartSet<T> &subject, const Predicate &pred) {
  for (const auto &element : subject) {
    pred(*element);
  }
}
} // namespace EFG
