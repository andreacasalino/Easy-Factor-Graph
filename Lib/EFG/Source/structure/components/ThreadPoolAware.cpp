/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef THREAD_POOL_ENABLED
#include <structure/components/ThreadPoolAware.h>

namespace EFG::strct {
    void ThreadPoolAware::setThreadPoolSize(const std::size_t& poolSize) {
		if (poolSize <= 1) {
			this->threadPool.reset();
			return;
		}

		if (nullptr != this->threadPool) {
			if (this->threadPool->size() == poolSize) return;
		}
		this->threadPool = std::make_unique<thpl::equi::Pool>(poolSize);
    }
}
#endif

