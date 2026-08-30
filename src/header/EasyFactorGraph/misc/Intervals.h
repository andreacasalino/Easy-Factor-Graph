/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/misc/Generator.h>
#include <EasyFactorGraph/misc/MemoryPool.h>

#include <algorithm>
#include <limits>
#include <span>
#include <utility>
#include <vector>

namespace EFG::misc {
struct Chunk {
  Chunk(std::size_t _begin, std::size_t _end, float _val)
      : begin{_begin}, end{_end}, val{_val} {
    if (end <= begin) {
      throw Error("Invalid chunk, begin: {} end: {}", begin, end);
    }
  }

  std::size_t begin;
  std::size_t end;
  float val;
};

class Intervals {
public:
  Intervals() = delete;

  using IntervalPoint = std::pair<std::size_t, float>;

  float get(std::size_t index) const;

  template <bool Sort>
  static Intervals from_points(std::vector<IntervalPoint> collection);

  template <bool Sort>
  static Intervals from_chunks(std::vector<Chunk> collection);

  template <typename G>
  requires IsGenerator<G, IntervalPoint>
  static Intervals from_gen(G gen);

  template <typename G>
  requires IsGenerator<G, Chunk>
  static Intervals from_gen(G gen);

  struct IntervalsIter {
    friend class Intervals;

    float next() {
      if (axis_position_ == source_[1].first) {
        source_ = {source_.begin() + 1, source_.end()};
      }
      axis_position_ += 1;
      return source_.front().second;
    }

  private:
    IntervalsIter(std::span<const IntervalPoint> source)
        : axis_position_{0}, source_{source} {}

    std::size_t axis_position_;
    std::span<const IntervalPoint> source_;
  };
  IntervalsIter iter() const { return IntervalsIter{delimiters.get()}; }

  const auto &getDelimiters() const { return delimiters; }

  void transferIntoPool(misc::MemoryPool &pool) {
    delimiters.transferIntoPool(pool);
  }

private:
  template <typename T>
  static Intervals from_vec(std::span<const T> collection);

  Intervals(std::vector<IntervalPoint> input);

  Slot<IntervalPoint> delimiters;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <bool Sort>
Intervals Intervals::from_points(std::vector<IntervalPoint> collection) {
  if constexpr (Sort) {
    std::sort(collection.begin(), collection.end(),
              [](const auto &a, const auto &b) { return a.first < b.first; });
  }
  return from_vec(std::span<const IntervalPoint>{collection});
}

template <bool Sort>
Intervals Intervals::from_chunks(std::vector<Chunk> collection) {
  if constexpr (Sort) {
    std::sort(collection.begin(), collection.end(),
              [](const auto &a, const auto &b) { return a.begin < b.begin; });
  }
  return from_vec(std::span<const Chunk>{collection});
}

template <typename T>
Intervals Intervals::from_vec(std::span<const T> collection) {
  using Gen = GenFromBlock<T>;
  return Intervals::from_gen<Gen>(Gen{collection});
}

template <typename G>
requires IsGenerator<G, Intervals::IntervalPoint>
    Intervals Intervals::from_gen(G gen) {
  using Gen = GeneratorMap<Chunk, IntervalPoint, G>;
  Gen gen_map{std::move(gen), [](const auto &inner) {
                return Chunk{inner.first, inner.first + 1, inner.second};
              }};
  return Intervals::from_gen<Gen>(std::move(gen_map));
}

template <typename G>
requires IsGenerator<G, Chunk> Intervals Intervals::from_gen(G gen) {
  std::vector<IntervalPoint> res;

  std::optional<Chunk> maybe_first = gen.next();
  if (!maybe_first.has_value()) {
    return res;
  }

  res.emplace_back(std::make_pair(maybe_first->begin, maybe_first->val));
  struct Prev {
    std::size_t end;
    float val;
  };
  Prev prev{maybe_first->end, maybe_first->val};

  while (true) {
    std::optional<Chunk> next = gen.next();
    if (!next.has_value()) {
      break;
    }

    if (next->begin < prev.end) {
      throw Error{"Invalid Chunks, make sure there are no collision or that "
                  "Chunk are "
                  "sorted in ascending order considering begin index"};
    }

    if (prev.end < next->begin) {
      // is not contiguous ... add gap of zeros if needed
      if (0 != prev.val) {
        // add gap of 0
        res.emplace_back(std::make_pair(prev.end, 0));
      }
      prev.end = next->begin;
      prev.val = 0;
    }

    if (prev.val != next->val) {
      res.emplace_back(std::make_pair(next->begin, next->val));
    }

    prev.end = next->end;
    prev.val = next->val;
  }

  if (prev.val != 0) {
    res.emplace_back(std::make_pair(prev.end, 0));
  }

  return {std::move(res)};
}

} // namespace EFG::misc
