/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_STRUCTURE_THPOOL_AWARE_H
#define EFG_STRUCTURE_THPOOL_AWARE_H

#ifdef THREAD_POOL_ENABLED
#include <Component.h>
#include <EquiPool.h>

namespace EFG::strct {
    class ThreadPoolAware : virtual public Component {
    public: 
        /** 
         * @brief Enable the thread pool. Use it only for medium-big sized graphs.
         *    - When passing  1, the pool is not used and an already enabled pool is destroyed. 
         *    - When passing  >= 1, the pool with the passed size is enabled and built internally 
         *    - When passing  0, the number of available cores on this machine is assumed as poolSize
         *    - When building the object, a default 1 size value is assumed, i.e. no thread pool is activated.
         */
        void setThreadPoolSize(const std::size_t& poolSize);

    protected:
        /**
         * @brief a thread pool that might be used in many ways to fasten some computations
         */
        std::unique_ptr<thpl::equi::Pool> threadPool;
    };
}
#endif

#endif
