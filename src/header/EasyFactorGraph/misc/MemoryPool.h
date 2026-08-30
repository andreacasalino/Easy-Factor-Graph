/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/Error.h>

#include <cstring>
#include <span>
#include <vector>

namespace EFG::misc {
class MemoryPool {
public:
  MemoryPool(std::size_t capacity_limit);

  ~MemoryPool() { delete[] buffer_; }

  MemoryPool(const MemoryPool &) = delete;
  MemoryPool &operator=(const MemoryPool &) = delete;
  MemoryPool(MemoryPool &&) noexcept = delete;
  MemoryPool &operator=(MemoryPool &&) noexcept = delete;

  template <typename T> std::span<T> cloneIntoPool(std::span<T> slot);

  template <typename T> std::span<T> claimFromPool(std::size_t size);

private:
  std::size_t len_;
  std::size_t capacity_;
  char *buffer_;
};

template <typename T> struct Slot {
  Slot() = default;

  Slot(const Slot &) = delete;
  Slot &operator=(const Slot &) = delete;

  Slot(Slot &&o) noexcept;
  Slot &operator=(Slot &&o) noexcept;

  ~Slot() {
    if (is_heap_allocated_) {
      delete[] block.data();
    }
  }

  static Slot makeNonOwning(std::span<T> the_view) {
    Slot res{0};
    res.block = the_view;
    return res;
  }

  static Slot makeOwning(std::span<const T> the_view) {
    Slot res{the_view.size()};
    std::memcpy(res.block.data(), the_view.data(), sizeof(T) * the_view.size());
    return res;
  }

  void transferIntoPool(misc::MemoryPool &pool);

  std::size_t getMemoryFootprint() const { return sizeof(T) * block.size(); }

  auto get() const { return block; }

private:
  Slot(std::size_t capacity) {
    if (0 < capacity) {
      block = {new T[capacity], capacity};
      is_heap_allocated_ = true;
    }
  }

  std::span<T> block;
  bool is_heap_allocated_{false};
};

class ValuesPool {
public:
  ValuesPool();

  template <typename SlotsSizesRng>
  static std::pair<ValuesPool, std::size_t> make(SlotsSizesRng rng) {
    std::pair<ValuesPool, std::size_t> res;
    res.second = 0;
    for (auto len : rng) {
      auto &added = res.first.slots_.emplace_back();
      added.first = res.second;
      res.second += len;
      added.second = res.second;
    }
    return res;
  }

  std::span<float> get_slot(std::size_t slot_idx, std::span<float> giver) {
    auto [begin, end] = slots_[slot_idx];
    return {giver.begin() + begin, giver.begin() + end};
  }

  std::span<const float> get_slot(std::size_t slot_idx,
                                  std::span<const float> giver) const {
    auto [begin, end] = slots_[slot_idx];
    return {giver.begin() + begin, giver.begin() + end};
  }

private:
  std::vector<std::pair<std::size_t, std::size_t>> slots_;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
std::span<T> MemoryPool::cloneIntoPool(std::span<T> slot) {
  auto claimed = this->template claimFromPool<T>(slot.size());
  std::memcpy(claimed.data(), slot.data(), sizeof(T) * slot.size());
  return claimed;
}

template <typename T> std::span<T> MemoryPool::claimFromPool(std::size_t size) {
  std::size_t residual = capacity_ - len_;
  std::size_t buffer_len = size * sizeof(T);
  if (residual < buffer_len) {
    throw Error{"Memory pool unexpectedly run out of memory"};
  }
  T *receiver = reinterpret_cast<T *>(buffer_ + len_);
  len_ += buffer_len;
  return {receiver, size};
}

template <typename T>
Slot<T>::Slot(Slot &&o) noexcept
    : block{o.block}, is_heap_allocated_{o.is_heap_allocated_} {
  o.block = {};
  o.is_heap_allocated_ = false;
}

template <typename T> Slot<T> &Slot<T>::operator=(Slot<T> &&o) noexcept {
  if (is_heap_allocated_) {
    delete[] block.data();
  }
  block = o.block;
  is_heap_allocated_ = o.is_heap_allocated_;
  o.block = {};
  o.is_heap_allocated_ = false;
  return *this;
}

template <typename T> void Slot<T>::transferIntoPool(misc::MemoryPool &pool) {
  auto new_block = pool.cloneIntoPool(block);
  if (is_heap_allocated_) {
    delete[] block.data();
  }
  is_heap_allocated_ = false;
  block = new_block;
}
} // namespace EFG::misc
