/**
 * Author:    Andrea Casalino
 * Created:   28.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/structure/components/PoolAware.h>

#include <atomic>
#include <mutex>
#include <thread>

namespace EFG::strct {
class WorkerConcrete : public Pool::Worker {
public:
  WorkerConcrete(const std::size_t threads_numb, const std::size_t thread_id);
  ~WorkerConcrete() override;

  void dispatch(const Tasks &tasks);

  bool isBusy() const { return is_busy; }

private:
  const std::size_t threads_numb;
  const std::size_t thread_id;
  std::atomic_bool life = true;
  std::atomic_bool is_busy = false;
  std::mutex tasks_mtx;
  const Tasks *tasks = nullptr;
  std::thread worker_;
};

namespace {
void process(const Tasks &subject, const std::size_t threads_numb,
             const std::size_t thread_id) {
  for (std::size_t k = thread_id; k < subject.size(); k += threads_numb) {
    subject[k](thread_id);
  }
}
} // namespace

WorkerConcrete::WorkerConcrete(const std::size_t threads_numb,
                               const std::size_t thread_id)
    : threads_numb(threads_numb), thread_id(thread_id) {
  std::atomic_bool not_started = true;
  worker_ = std::thread{[this, &not_started]() {
    not_started = false;
    while (this->life) {
      std::scoped_lock lock(this->tasks_mtx);
      if (nullptr != this->tasks) {
        this->is_busy = true;
        process(*this->tasks, this->threads_numb, this->thread_id);
        this->tasks = nullptr;
        this->is_busy = false;
      }
    }
  }};
  while (not_started) {
  }
}

WorkerConcrete::~WorkerConcrete() {
  life = false;
  worker_.join();
}

void WorkerConcrete::dispatch(const Tasks &tasks) {
  std::scoped_lock lock(this->tasks_mtx);
  this->tasks = &tasks;
}

Pool::Pool(const std::size_t size) {
  if (0 == size) {
    throw Error{"Invalid Pool size"};
  }
  workers.reserve(size - 1);
  for (std::size_t k = 1; k < size; ++k) {
    workers.emplace_back(std::make_unique<WorkerConcrete>(size, k));
  }
}

void Pool::parallelFor(const std::vector<Task> &tasks) {
  std::scoped_lock(parallel_for_dispatch_mtx);
  for (auto &worker : workers) {
    static_cast<WorkerConcrete *>(worker.get())->dispatch(tasks);
  }
  process(tasks, workers.size() + 1, 0);
  for (auto &worker : workers) {
    while (static_cast<WorkerConcrete *>(worker.get())->isBusy()) {
    }
  }
}

PoolAware::PoolAware() { resetPool(); }

PoolAware::~PoolAware() = default;

void PoolAware::resetPool() { pool = std::make_unique<Pool>(1); }

void PoolAware::setPoolSize(const std::size_t new_size) {
  std::scoped_lock(pool_mtx);
  if (new_size == pool->size()) {
    return;
  }
  pool = std::make_unique<Pool>(new_size);
}
} // namespace EFG::strct
