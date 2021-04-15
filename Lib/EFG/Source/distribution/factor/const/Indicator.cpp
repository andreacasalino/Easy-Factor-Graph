


/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/factor/const/Indicator.h>

namespace EFG::distribution::factor::cnst {
    IndicatorFactor::IndicatorFactor(categoric::VariablePtr var, std::size_t evidence)
        : Factor(categoric::Group(var)) {
        this->values->emplace(Combination({ evidence }), 1.f);
    };
}
