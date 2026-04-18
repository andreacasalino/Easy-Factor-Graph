#include <EasyFactorGraph/structure/WorkerPoolManager.h>

namespace EFG::structure {
WorkerPoolManager::ScopedActivator::ScopedActivator(WorkerPoolManager &source)
    : source_{source} {
  auto &pool =
      source_.workers_.emplace(source_.context_->config.workers_pool_size);
  for (auto *l : source_.context_->getListeners()) {
    l->updatePool(&pool);
  }
}

WorkerPoolManager::ScopedActivator::~ScopedActivator() {
  for (auto *l : source_.context_->getListeners()) {
    l->updatePool(nullptr);
  }
  source_.workers_.reset();
}
} // namespace EFG::structure
