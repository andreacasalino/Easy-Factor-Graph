/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <observers/Observer.h>

namespace EFG {
    Observer::Observer(const Subject& subject) {
        this->counter = subject.counter;
    };

    Observer::~Observer(){
        this->counter->rmObserver();
    };
}
