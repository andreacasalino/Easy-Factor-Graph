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
        /** \brief Enable the thread pool. Use for big graphs.
         * IMPORTANT!!! If the compile flag THREAD_POOL_ENABLED was not defined, calling this function will have no effect.
         * When passing <= 1 the actual pool is destroyed. When building the object, a defualt 0 size value is assumed
         * @return true in case the pool was successfully set.
         */
        void SetThreadPoolSize(const std::size_t& poolSize);

    protected:
        std::unique_ptr<thpl::equi::Pool> threadPool;
    };
}
#endif

#endif
