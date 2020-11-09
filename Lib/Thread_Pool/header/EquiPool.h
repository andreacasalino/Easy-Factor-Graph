/**
* Author:    Andrea Casalino
* Created:   25.09.2020
*
* report any bug to andrecasa91@gmail.com.
**/

#ifndef __POOL_EQUI_H__
#define __POOL_EQUI_H__

#include "ThreadPool.h"

namespace thpl::equi {

    /**
     * \brief Tasks are ordered in a FIFO list
     */
    class Pool : public IPool {
    public:

    /**
     * \brief Constructor
     * @param[in] poolSize the number of threads to spawn that will be part of the pool
     */
    Pool(const std::size_t& poolSize);

    ~Pool() override;

    /**
     * @param[in] newTask the newer task to add to the FIFO queue
     */
    void push(const std::function<void(void)>& newTask);

    private:

        class EquiQueue;
        EquiQueue* Q;

    };

}

#endif