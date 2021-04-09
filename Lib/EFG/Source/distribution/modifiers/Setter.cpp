/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/modifiers/Setter.h>
//#include <categoric/Range.h>
//#include <Error.h>
//#include <algorithm>

namespace EFG::distribution {
    void Setter::replaceGroup(const categoric::Group& newGroup) {
        this->checkModifiable();
        *this->group = newGroup;
    }

    void Setter::set(const Combination& comb, const float& value) {
        this->checkModifiable();
        this->checkCombination(comb, value);
        auto it = this->values->find(comb);
        if (it == this->values->end()) {
            // add the combination and set it
            auto emplaced = this->values->emplace(comb, value);
            emplaced.first->second = value;
        }
        it->second = value;
    }
}
