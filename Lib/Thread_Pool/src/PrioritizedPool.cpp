/**
* Author:    Andrea Casalino
* Created:   25.09.2020
*
* report any bug to andrecasa91@gmail.com.
**/

#include "../header/PrioritizedPool.h"
#include "QueueStrategy.h"
#include <queue>
#include <utility>

namespace thpl::prty{

    class Pool::PriorityQueue : public IPool::QueueStrategy {
    public:
        PriorityQueue();

        inline bool isEmpty() override{ return this->queue.empty(); };

        inline std::function<void(void)> pop() override{
            std::function<void(void)> temp = std::move(this->queue.top().second);
            this->queue.pop();
            return temp;
        };

        inline void push(const std::function<void(void)>& task, const unsigned int& pr) { this->queue.push(std::make_pair(pr, task)); };
    private:
        typedef std::pair<unsigned int, std::function<void(void)>> task;
        std::priority_queue< task, std::vector<task>, std::function<bool(const task&, const task&)>> queue;
    };

    Pool::PriorityQueue::PriorityQueue()
        : queue([](const task& a, const task& b){ return a.first < b.first; }) {
    };

    Pool::Pool(const std::size_t& poolSize)
        : IPool(poolSize, std::make_unique<PriorityQueue>()) {
    }

    void Pool::push(const std::function<void(void)>& newTask, const unsigned int& priority){
        {
            std::lock_guard<std::mutex> lk(this->queueMtx);
            static_cast<PriorityQueue*>(this->queue.get())->push(newTask, priority);
        }
        ++this->remainingTasks;
    }

}