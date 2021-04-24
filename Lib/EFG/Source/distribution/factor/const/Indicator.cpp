


/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/factor/const/Indicator.h>
#include <Error.h>

namespace EFG::distribution::factor::cnst {
    IndicatorFactor::IndicatorFactor(categoric::VariablePtr var, std::size_t evidence)
        : Factor({ var }) {
        if(evidence >= var->size()) {
            throw Error("invalid evidence");
        }
        this->values->emplace(categoric::Combination(&evidence, 1), 1.f);
    };
}
