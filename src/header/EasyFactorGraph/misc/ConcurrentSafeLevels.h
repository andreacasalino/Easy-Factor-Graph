/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/Error.h>

#include <algorithm>
#include <span>
#include <unordered_set>
#include <vector>

namespace EFG::misc {
template <typename T> struct ConcurrentSafeLevels {
  ConcurrentSafeLevels() = default;

  template <typename Pred> void for_each(Pred &&pred) const {
    std::for_each(support_.begin(), support_.end(), std::forward<Pred>(pred));
  }

  template <typename IterT> void add(IterT begin, IterT end);

  template <typename Pred> void for_each_level(Pred pred) const;

private:
  std::vector<T> support_;

  // connections in the same level, are related to messages
  // that can be concurrently updated
  std::vector<std::size_t> levels_sizes_;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
template <typename IterT>
void ConcurrentSafeLevels<T>::add(IterT begin, IterT end) {
  std::size_t len = 0;
  std::for_each(begin, end, [&](auto el) {
    support_.push_back(el);
    len += 1;
  });
  if (0 < len) {
    levels_sizes_.push_back(len);
  }
}

template <typename T>
template <typename Pred>
void ConcurrentSafeLevels<T>::for_each_level(Pred pred) const {
  std::size_t cursor{0};
  for (auto level_size : levels_sizes_) {
    auto it_begin = support_.begin() + cursor;
    auto it_end = it_begin + level_size;
    cursor += level_size;
    pred(std::span<const T>{it_begin, it_end});
  }
}
} // namespace EFG::misc
