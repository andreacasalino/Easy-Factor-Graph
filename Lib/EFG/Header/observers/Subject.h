/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_SUBJECT_H
#define EFG_SUBJECT_H

#include <observers/ObserverCounter.h>

namespace EFG {
    class Subject {
    friend class Observer;
    public:
        virtual ~Subject();
        
    protected:
        Subject(ObserverCounterPtr counter);

    private:
        ObserverCounterPtr counter;
    };
}

#endif
