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

        //spawn all the threads in the pool
        for(std::size_t k=0; k< poolSize; ++k)
            this->pool.emplace_back([this]() {
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
                        this->oneTaskFinishedBarrier.second.notify_all();
                    }
                    else {
                        // wait till at least one task is inserted or destructor is called
                        std::unique_lock<std::mutex> lk(this->newTaskReadyBarrier.first);
                        this->newTaskReadyBarrier.second.wait_for(lk, std::chrono::seconds(1));
                    }
                }
        });
    }

    void IPool::newTaskReady(){
        ++this->remainingTasks;
        this->newTaskReadyBarrier.second.notify_one();
    }

    void IPool::wait(){
        while (true) {
            if(this->remainingTasks == 0) break;

            // wait for another 1 second that least one thread notify finish task
            std::unique_lock<std::mutex> lk(this->oneTaskFinishedBarrier.first);
            this->oneTaskFinishedBarrier.second.wait_for(lk, std::chrono::seconds(1));
        }
        
    }

    IPool::~IPool(){
        this->poolLife = false;
        this->newTaskReadyBarrier.second.notify_all();
        std::for_each(this->pool.begin(), this->pool.end(), [](std::thread& t) { t.join(); });
    }

}