/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_SUBJECT_MULTI_COUNTER_H
#define EFG_SUBJECT_MULTI_COUNTER_H

#include <observers/ObserverCounter.h>
#include <atomic>

namespace EFG {
    class SubjectMultiCounter : public ObserverCounter {
    public:
        inline void addObserver() final { ++this->counter; };
        void rmObserver() final { --this->counter; };
        bool isObserved() const final { return (this->counter > 0); };

    private:
        std::atomic<std::size_t> counter = 0;
    };
}

#endif
