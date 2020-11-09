/**
* Author:    Andrea Casalino
* Created:   25.09.2020
*
* report any bug to andrecasa91@gmail.com.
**/
#ifndef __QUEUE_STRATEGY_H__
#define __QUEUE_STRATEGY_H__

#include "../header/ThreadPool.h"

namespace thpl{

    class IPool::QueueStrategy{
    public:
        virtual bool isEmpty() = 0;

        virtual std::function<void(void)> pop() = 0;
    };

}

#endif