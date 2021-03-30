/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <observers/Observer.h>

namespace EFG::observer {
    Observer::Observer(const Subject& subject) {
        this->counter = subject.counter;
        this->counter->addObserver();
    };

    Observer::~Observer(){
        this->counter->rmObserver();
    };
}
