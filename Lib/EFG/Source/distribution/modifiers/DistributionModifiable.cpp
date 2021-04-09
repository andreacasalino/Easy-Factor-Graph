/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/modifiers/DistributionModifiable.h>
#include <Error.h>
#include <algorithm>

namespace EFG::distribution {
    void DistributionModifiable::checkCombination(const Combination& comb, const float& value) const {
        if(value < 0.f) {
            throw Error("negative value is invalid");
        }
        if(comb.size() != this->getGroup().getVariables().size()) {
            throw Error("invalid combination size");
        }
        std::size_t k = 0;
        std::for_each(this->getGroup().getVariables().begin(), this->getGroup().getVariables().end(), [&k, &comb](const categoric::VariablePtr& v){
            if(comb.data()[k] >= v->size()) {
                throw Error("combination value exceed variable domain size");
            }
            ++k;
        });
    }

    void DistributionModifiable::checkModifiable() const {
        if(this->isObserved()) {
            throw Error("distribution can't be changed");
        }
    }
}
