/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_OBSERVER_COUNTER_H
#define EFG_OBSERVER_COUNTER_H

#include <memory>

namespace EFG {
    class ObserverCounter {
    public:
        virtual void addObserver() = 0;
        virtual void rmObserver() = 0;
        virtual bool isObserved() const = 0;
    };

    typedef std::shared_ptr<ObserverCounter> ObserverCounterPtr;
}

#endif
