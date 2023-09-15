/**
 * Author:    Andrea Casalino
 * Created:   28.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

namespace EFG::strct {
using Task = std::function<void(const std::size_t)>; // the processing thread id
                                                     // is passed
using Tasks = std::vector<Task>;

class Pool {
public:
  Pool(std::size_t size);
  ~Pool();

  void parallelFor(const Tasks &tasks);

  std::size_t size() const { return ctxt.pool_size; }

private:
  struct Context {
    std::size_t pool_size;
    std::atomic_bool life = true;
  };
  Context ctxt;

  std::mutex parallelForMtx;

  struct Worker {
    Worker(std::size_t th_id, Context &context);

    std::thread loop;

    std::atomic<const Tasks *> to_process = nullptr;
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

  Pool &getPool() { return pool.value(); }
  void setPoolSize(std::size_t new_size);

  class ScopedPoolActivator {
  public:
    ScopedPoolActivator(PoolAware &subject, std::size_t new_size)
        : subject(subject) {
      subject.setPoolSize(new_size);
    }
    ~ScopedPoolActivator() { subject.resetPool(); }

  private:
    PoolAware &subject;
  };

private:
  std::optional<Pool> pool;
};
} // namespace EFG::strct
