/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <barrier>
#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <stop_token>
#include <thread>
#include <vector>

namespace EFG::misc {
class WorkerPool {
public:
  explicit WorkerPool(std::size_t pool_size);

  ~WorkerPool();

  WorkerPool(const WorkerPool &) = delete;
  WorkerPool &operator=(const WorkerPool &) = delete;

  template <typename Pred> void compute(Pred pred) {
    command_ = pred;
    barrier_.arrive_and_wait();
    pred(0);
    barrier_.arrive_and_wait();
    command_ = {};
  }

  [[nodiscard]] std::size_t size() const noexcept {
    return workers_.size() + 1;
  }

private:
  void run_worker(std::size_t th_id, std::stop_token st);

  std::stop_source stop_src_;
  // TODO replace with a variant of up-front declared possible commands
  std::function<void(std::size_t)> command_;
  std::barrier<> barrier_;
  std::vector<std::jthread> workers_;
};

} // namespace EFG::misc