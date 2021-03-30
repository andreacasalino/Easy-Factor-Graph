/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <observers/UniqueObservable.h>
#include "UniqueCounter.h"

namespace EFG::observer {
    UniqueObservable::UniqueObservable() 
        : Subject(std::make_shared<UniqueCounter>()) {
    }
}
