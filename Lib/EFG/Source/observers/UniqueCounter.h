/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_OBSERVER_UNIQUE_COUNTER_H
#define EFG_OBSERVER_UNIQUE_COUNTER_H

#include <observers/Counter.h>
#include <Error.h>
#include <atomic>

namespace EFG::observer {
    class UniqueCounter : public Counter {
    public:
        inline void addObserver() final { if(this->counter) throw Error("SubjectUniqueCounter", "subject already observed"); this->counter = true; };
        void rmObserver() final { this->counter = false; };
        bool isObserved() const final { return this->counter; };

    private:
        std::atomic_bool counter = false;
    };
}

#endif
