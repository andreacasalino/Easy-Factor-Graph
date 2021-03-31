/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_OBSERVER_SUBJECT_H
#define EFG_OBSERVER_SUBJECT_H

#include <observers/Counter.h>

namespace EFG::observer {
    class Subject {
    friend class Observer;
    public:
        virtual ~Subject();

        Subject(const Subject&) = delete;
        Subject& operator=(const Subject&) = delete;

        inline bool isObserved() const { return this->counter->isObserved(); };
        
    protected:
        Subject(CounterPtr counter);

    private:
        CounterPtr counter;
    };
}

#endif
