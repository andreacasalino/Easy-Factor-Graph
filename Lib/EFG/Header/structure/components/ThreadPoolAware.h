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
         * When passing <= 1 the actual pool is destroyed. When building the object, a default 0 size value is assumed,
         * i.e. no thread pool is activated.
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
