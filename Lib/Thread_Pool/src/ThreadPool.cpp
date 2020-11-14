/**
* Author:    Andrea Casalino
* Created:   25.09.2020
*
* report any bug to andrecasa91@gmail.com.
**/

#include "QueueStrategy.h"
#include <algorithm>

namespace thpl{

    IPool::IPool(const std::size_t& poolSize, std::unique_ptr<QueueStrategy> queue)
        : queue(std::move(queue))
        , poolLife(true)
        , remainingTasks(0) {
        if(poolSize == 0) throw std::runtime_error("invalid pool size");

        std::atomic<std::size_t> spawnedReady = 0;

        //spawn all the threads in the pool
        for(std::size_t k=0; k< poolSize; ++k)
            this->pool.emplace_back([this, &spawnedReady]() {
                ++spawnedReady;
                bool isEmpty;
                while (this->poolLife) {
                    std::function<void(void)> task;
                    {
                        std::lock_guard<std::mutex> lk(this->queueMtx);
                        isEmpty = this->queue->isEmpty();
                        if (!isEmpty) task = this->queue->pop();
                    }
                    if (!isEmpty) {
                        task();
                        --this->remainingTasks;
                    }
                }
        });

        // make sure all the threads were spawned before returning
        while (true) {
            if (spawnedReady == poolSize) break;
        }
    }

    void IPool::wait(){
        while (true) {
            if (this->remainingTasks == 0) return;
        }
    }

    IPool::~IPool(){
        this->poolLife = false;
        std::for_each(this->pool.begin(), this->pool.end(), [](std::thread& t) { t.join(); });
    }

}