/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <type_traits>

namespace EFG {
template <typename DownCastT, typename T, typename Predicate>
void castConstAndUse(const T &subject, const Predicate &predicate) {
  if (auto *ptr = dynamic_cast<const DownCastT *>(&subject); ptr) {
    predicate(*ptr);
  }
}

template <typename DownCastT, typename T, typename Predicate>
void castAndUse(T &subject, const Predicate &predicate) {
  if (auto *ptr = dynamic_cast<DownCastT *>(&subject); ptr) {
    predicate(*ptr);
  }
}
} // namespace EFG
