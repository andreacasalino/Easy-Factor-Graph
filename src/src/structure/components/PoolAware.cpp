/**
 * Author:    Andrea Casalino
 * Created:   28.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/components/PoolAware.h>

namespace EFG::strct {
namespace {
void process_tasks_in_parallel(const Tasks &subject,
                               const std::size_t threads_numb,
                               const std::size_t thread_id) {
  for (std::size_t k = thread_id; k < subject.size(); k += threads_numb) {
    subject[k](thread_id);
  }
}
} // namespace

Pool::Worker::Worker(const std::size_t th_id, WorkersContext &context)
    : loop([thread_id = th_id, &context = context, this]() {
        while (context.life) {
          if (!this->has_something_to_process) {
            continue;
          }
          process_tasks_in_parallel(*this->to_process, context.pool_size,
                                    thread_id);
          this->to_process = nullptr;
          this->has_something_to_process = false;
        }
      }) {}

Pool::Pool(const std::size_t size) {
  if (0 == size) {
    throw Error{"Invalid Pool size"};
  }
  workers_context.pool_size = size;
  for (std::size_t k = 1; k < size; ++k) {
    workers.emplace_back(std::make_unique<Worker>(k, workers_context));
  }
}

Pool::~Pool() {
  workers_context.life = false;
  for (auto &worker : workers) {
    // if (worker->loop.joinable()) {
    worker->loop.join();
    // }
  }
  workers.clear();
}

void Pool::parallelFor(const std::vector<Task> &tasks) {
  std::scoped_lock lock(parallel_for_dispatch_mtx);
  for (auto &worker : workers) {
    worker->to_process = &tasks;
    worker->has_something_to_process = true;
  }
  process_tasks_in_parallel(tasks, workers_context.pool_size, 0);
  bool keep_wait;
  do {
    keep_wait = false;
    for (const auto &worker : workers) {
      if (worker->has_something_to_process) {
        keep_wait = true;
        break;
      }
    }
  } while (keep_wait);
}

PoolAware::PoolAware() { resetPool(); }

PoolAware::~PoolAware() = default;

void PoolAware::resetPool() { pool = std::make_unique<Pool>(1); }

void PoolAware::setPoolSize(const std::size_t new_size) {
  if (new_size == pool->size()) {
    return;
  }
  pool = std::make_unique<Pool>(new_size);
}
} // namespace EFG::strct
