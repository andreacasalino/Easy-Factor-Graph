/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/misc/WorkerPool.h>

namespace EFG::misc {
WorkerPool::WorkerPool(std::size_t pool_size) : barrier_(pool_size) {
  workers_.reserve(pool_size - 1);
  for (std::size_t th_id = 1; th_id < pool_size; ++th_id) {
    workers_.emplace_back(
        [this, th_id, st = stop_src_.get_token()]() { runWorker(th_id, st); });
  }
}

WorkerPool::~WorkerPool() {
  stop_src_.request_stop();
  barrier_.arrive_and_wait();
}

void WorkerPool::runWorker(std::size_t th_id, std::stop_token st) {
  while (true) {
    barrier_.arrive_and_wait();
    if (st.stop_requested()) {
      break;
    }
    command_(th_id);
    barrier_.arrive_and_wait();
  }
}

std::vector<std::pair<std::size_t, std::size_t>>
per_threads_indices_subdivision(std::size_t threads, std::size_t len) {
  std::vector<std::pair<std::size_t, std::size_t>> res;

  std::size_t len_per_thread = len / threads;
  std::size_t offset{0};
  auto move_by_ = [&](std::size_t len) {
    auto res = std::make_pair(offset, offset + len);
    offset += len;
    return res;
  };

  if (len <= threads) {
    for (std::size_t t = 0; t < len; ++t) {
      res.push_back(move_by_(1));
    }
    while (res.size() < threads) {
      res.emplace_back(move_by_(0));
    }
    return res;
  }

  std::size_t rest = len % threads;
  for (std::size_t r = 0; r < rest; ++r) {
    res.emplace_back(move_by_(len_per_thread + 1));
  }
  while (res.size() < threads) {
    res.emplace_back(move_by_(len_per_thread));
  }
  return res;
}
} // namespace EFG::misc
