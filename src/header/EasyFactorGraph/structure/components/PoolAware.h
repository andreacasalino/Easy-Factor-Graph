/**
 * Author:    Andrea Casalino
 * Created:   28.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace EFG::strct {
using Task = std::function<void(const std::size_t)>; // the processing thread id
                                                     // is passed
using Tasks = std::vector<Task>;

class Pool {
public:
  Pool(const std::size_t size);
  ~Pool();

  void parallelFor(const std::vector<Task> &tasks);

  std::size_t size() const { return workers_context.pool_size; }

private:
  std::mutex parallel_for_dispatch_mtx;

  struct WorkersContext {
    std::size_t pool_size;
    std::atomic_bool life = true;
  };
  WorkersContext workers_context;
  struct Worker {
    Worker(const std::size_t th_id, WorkersContext &context);

    std::thread loop;

    std::atomic_bool has_something_to_process = false;
    const Tasks *to_process = nullptr;
  };
  using WorkerPtr = std::unique_ptr<Worker>;
  std::vector<WorkerPtr> workers;
};

class PoolAware {
public:
  virtual ~PoolAware();

protected:
  PoolAware();

  void resetPool();

  Pool &getPool() { return *pool; }
  void setPoolSize(const std::size_t new_size);

  class ScopedPoolActivator {
  public:
    ScopedPoolActivator(PoolAware &subject, const std::size_t new_size)
        : subject(subject) {
      subject.setPoolSize(new_size);
    }
    ~ScopedPoolActivator() { subject.resetPool(); }

  private:
    PoolAware &subject;
  };

private:
  std::unique_ptr<Pool> pool;
};
} // namespace EFG::strct
