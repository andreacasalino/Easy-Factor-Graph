/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <observers/Subject.h>
#include <Error.h>

namespace EFG {
    Subject::Subject(ObserverCounterPtr counter)
        : counter(counter) {
    }

    Subject::~Subject() {
        if(this->counter->isObserved()) {
            throw Error("Subject", "not all observers were relased before destruction");
        }
    }
}
