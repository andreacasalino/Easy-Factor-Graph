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
        [this, th_id, st = stop_src_.get_token()]() { run_worker(th_id, st); });
  }
}

WorkerPool::~WorkerPool() {
  stop_src_.request_stop();
  barrier_.arrive_and_wait();
}

void WorkerPool::run_worker(std::size_t th_id, std::stop_token st) {
  while (true) {
    barrier_.arrive_and_wait();
    if (st.stop_requested()) {
      break;
    }
    command_(th_id);
    barrier_.arrive_and_wait();
  }
}
} // namespace EFG::misc
