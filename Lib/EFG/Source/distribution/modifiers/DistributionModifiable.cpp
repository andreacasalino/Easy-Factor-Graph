/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/modifiers/DistributionModifiable.h>
#include <Error.h>

namespace EFG::distribution {
    void DistributionModifiable::checkModifiable() const {
        if(this->isObserved()) {
            throw Error("distribution can't be changed");
        }
    }
}
