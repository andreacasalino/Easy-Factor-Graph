/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_OBSERVER_UNIQUE_OBSERVABLE_H
#define EFG_OBSERVER_UNIQUE_OBSERVABLE_H

#include <observers/Subject.h>

namespace EFG::observer {
    class UniqueObservable : Subject {
    protected:
        UniqueObservable();
    };
}

#endif
