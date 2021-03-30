/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_OBSERVER_MULTI_COUNTER_H
#define EFG_OBSERVER_MULTI_COUNTER_H

#include <observers/Counter.h>
#include <atomic>

namespace EFG::observer {
    class MultiCounter : public Counter {
    public:
        inline void addObserver() final { ++this->counter; };
        void rmObserver() final { --this->counter; };
        bool isObserved() const final { return (this->counter > 0); };

    private:
        std::atomic<std::size_t> counter = 0;
    };
}

#endif
