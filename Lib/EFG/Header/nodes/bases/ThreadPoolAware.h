/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_THPOOL_AWARE_H
#define EFG_NODES_THPOOL_AWARE_H

#ifdef THREAD_POOL_ENABLED
#include <nodes/bases/Base.h>
#include <EquiPool.h>

namespace EFG::nodes {
    class ThreadPoolAware : virtual public Base {
    public: 
        /** 
         * @brief Enable the thread pool. Use it only for medium-big sized graphs.
         * When passing <= 1 the actual pool is destroyed. When building the object, a default 0 size value is assumed,
         * i.e. no thread pool is activated.
         */
        void SetThreadPoolSize(const std::size_t& poolSize);

    protected:
        /**
         * @brief a thread pool that might be used in many ways to fasten some computations
         */
        std::unique_ptr<thpl::equi::Pool> threadPool;
    };
}
#endif

#endif
