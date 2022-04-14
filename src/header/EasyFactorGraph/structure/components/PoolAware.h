/**
 * Author:    Andrea Casalino
 * Created:   28.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace EFG::strct {
using Task = std::function<void(const std::size_t)>; // the processing thread id
                                                     // is passed
using Tasks = std::vector<Task>;

class Pool {
public:
  Pool(const std::size_t size);

  void parallelFor(const std::vector<Task> &tasks);

  std::size_t size() const { return workers.size() + 1; }

  class Worker {
  public:
    virtual ~Worker() = default;

  protected:
    Worker() = default;
  };

private:
  using WorkerPtr = std::unique_ptr<Worker>;

  std::mutex parallel_for_dispatch_mtx;

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
  std::mutex pool_mtx;
  std::unique_ptr<Pool> pool;
};
} // namespace EFG::strct
