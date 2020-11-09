/**
* Author:    Andrea Casalino
* Created:   25.09.2020
*
* report any bug to andrecasa91@gmail.com.
**/

#include "QueueStrategy.h"
#include <algorithm>

namespace thpl{

    IPool::IPool(const std::size_t& poolSize, IPool::QueueStrategy* queue) 
        : poolSize(poolSize)
        , queue(queue)
        , poolLife(true)
        , remainingTasks(0) {
        if(this->poolSize == 0) throw std::runtime_error("invalid pool size");

        //spawn all the threads in the pool
        for(std::size_t k=0; k<this->poolSize; ++k)
            this->pool.emplace_back(&IPool::threadTask, this);
    }

    IPool::QueueStrategy* IPool::getQueue(){
        return this->queue;
    }

    void IPool::notifyTaskInsertion(){
        ++this->remainingTasks;
        this->insertionBarrier.notify_one();
    }

    void IPool::threadTask(){
        bool isEmpty;
        while (this->poolLife) {
            std::function<void(void)> task;
            {
                std::lock_guard<std::mutex> lk(this->insertionMtx);
                isEmpty = this->queue->isEmpty();
                if(!isEmpty) task = std::move(this->queue->pop());
            }
            if(!isEmpty) {
                task();
                --this->remainingTasks;
                this->finishBarrier.notify_one();
            }
            else{
            // wait till at least one task is inserted or destructor is called
                std::unique_lock<std::mutex> lk(this->insertionBarrierMtx);
                this->insertionBarrier.wait(lk);
            }
        }
        
    }

    void IPool::wait(){
        size_t remaining;
        while (true) {
            remaining = this->remainingTasks;
            if(remaining == 0) break;

            //wait till at least one thread notify finish task
            std::unique_lock<std::mutex> lk(this->finishBarrierMtx);
            this->finishBarrier.wait(lk);
        }
        
    }

    IPool::~IPool(){
        this->poolLife = false;
        this->insertionBarrier.notify_all();
        std::for_each(this->pool.begin(), this->pool.end(), [](std::thread& t){ t.join(); });
    }

}