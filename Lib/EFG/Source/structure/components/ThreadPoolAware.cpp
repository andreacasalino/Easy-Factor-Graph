/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef THREAD_POOL_ENABLED
#include <structure/components/ThreadPoolAware.h>
#include <omp.h>

namespace EFG::strct {
	std::size_t getAvailableCores() {
		int cores;
#pragma omp parallel
		{
			if (omp_get_thread_num() == 0) {
				cores = omp_get_num_threads();
			}
		}
		return static_cast<std::size_t>(cores);
	}

    void ThreadPoolAware::setThreadPoolSize(const std::size_t& poolSize) {
		std::size_t size = poolSize;
		if (0 == poolSize) {
			size = getAvailableCores();
		}

		if (1 == size) {
			this->threadPool.reset();
			return;
		}

		if (nullptr != this->threadPool) {
			if (this->threadPool->size() == size) return;
		}
		this->threadPool = std::make_unique<thpl::equi::Pool>(size);
    }
}
#endif

