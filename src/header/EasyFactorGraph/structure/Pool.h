/**
 * Author:    Andrea Casalino
 * Created:   28.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace EFG::strct {
using Task = std::function<void()>;
using Tasks = std::vector<Task>;

class Pool {
public:
  Pool(const std::size_t size);

  void parallelFor(const std::vector<Task> &tasks);

  std::size_t size() const { return workers.size() + 1; }

private:
  class Worker;
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

private:
  std::unique_ptr<Pool> pool;
};
} // namespace EFG::strct
