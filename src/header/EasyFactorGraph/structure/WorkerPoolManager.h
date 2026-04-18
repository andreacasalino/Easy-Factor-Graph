/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/misc/WorkerPool.h>
#include <EasyFactorGraph/structure/Structure.h>

#include <memory>
#include <optional>

namespace EFG::structure {
class WorkerPoolManager {
public:
  struct ScopedActivator {
    friend class WorkerPoolManager;

    ~ScopedActivator();

    ScopedActivator(const ScopedActivator &) = delete;
    ScopedActivator &operator=(const ScopedActivator &) = delete;
    ScopedActivator(ScopedActivator &&) noexcept = delete;
    ScopedActivator &operator=(ScopedActivator &&) noexcept = delete;

    misc::WorkerPool &getWorkers() { return source_.workers_.value(); }

  private:
    ScopedActivator(WorkerPoolManager &source);

    WorkerPoolManager &source_;
  };
  std::unique_ptr<ScopedActivator> activatePool() {
    std::unique_ptr<ScopedActivator> res;
    res.reset(new ScopedActivator{*this});
    return res;
  }

protected:
  void init(StructurePtr context) { context_ = context; }

private:
  std::optional<misc::WorkerPool> workers_;
  StructurePtr context_;
};
} // namespace EFG::structure
