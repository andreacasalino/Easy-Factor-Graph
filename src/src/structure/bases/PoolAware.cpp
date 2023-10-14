/**
 * Author:    Andrea Casalino
 * Created:   28.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/bases/PoolAware.h>

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

Pool::Worker::Worker(std::size_t th_id, Context &context)
    : loop([thread_id = th_id, &context = context, this]() {
        while (context.life.load()) {
          if (const Tasks *ptr = to_process.load(); ptr != nullptr) {
            process_tasks_in_parallel(*ptr, context.pool_size, thread_id);
            to_process.store(nullptr);
          }
        }
      }) {}

Pool::Pool(const std::size_t size) {
  if (0 == size) {
    throw Error{"Invalid Pool size"};
  }
  ctxt.pool_size = size;
  for (std::size_t k = 1; k < size; ++k) {
    workers.emplace_back(std::make_unique<Worker>(k, ctxt));
  }
}

Pool::~Pool() {
  ctxt.life.store(false);
  for (auto &worker : workers) {
    // if (worker->loop.joinable()) {
    worker->loop.join();
    // }
  }
  workers.clear();
}

void Pool::parallelFor(const std::vector<Task> &tasks) {
  std::scoped_lock lock(parallelForMtx);
  for (auto &worker : workers) {
    worker->to_process.store(&tasks);
  }
  process_tasks_in_parallel(tasks, ctxt.pool_size, 0);
  bool keep_wait;
  do {
    keep_wait = false;
    for (const auto &worker : workers) {
      if (worker->to_process.load() != nullptr) {
        keep_wait = true;
        break;
      }
    }
  } while (keep_wait);
}

PoolAware::PoolAware() { resetPool(); }

PoolAware::~PoolAware() = default;

void PoolAware::resetPool() { pool.emplace(1); }

void PoolAware::setPoolSize(const std::size_t new_size) {
  if (new_size == pool->size()) {
    return;
  }
  pool.emplace(new_size);
}
} // namespace EFG::strct
