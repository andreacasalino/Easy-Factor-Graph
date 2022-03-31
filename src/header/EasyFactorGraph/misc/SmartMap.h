/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include "SmartPointerUtils.h"

#include <unordered_map>

namespace EFG {
template <typename K, typename V>
using SmartMap =
    std::unordered_map<std::shared_ptr<K>, V, Hasher<K>, Comparator<K>>;

template <typename K, typename V, typename Predicate>
void for_each(SmartMap<K, V> &subject, const Predicate &pred) {
  for (auto it = subject.begin(); it != subject.end(); ++it) {
    pred(*it->first, it->second);
  }
};

template <typename K, typename V, typename Predicate>
void for_each(const SmartMap<K, V> &subject, const Predicate &pred) {
  for (auto it = subject.begin(); it != subject.end(); ++it) {
    pred(*it->first, it->second);
  }
};
} // namespace EFG
