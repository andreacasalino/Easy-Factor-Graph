/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <observers/MultiObservable.h>
#include "MultiCounter.h"

namespace EFG::observer {
    MultiObservable::MultiObservable() 
        : Subject(std::make_shared<MultiCounter>()) {
    }
}
