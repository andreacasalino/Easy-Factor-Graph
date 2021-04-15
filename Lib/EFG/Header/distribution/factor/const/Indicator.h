/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_FACTOR_INDICATOR_H
#define EFG_DISTRIBUTION_FACTOR_INDICATOR_H

#include <distribution/factor/const/Factor.h>

namespace EFG::distribution::factor::cnst {
    class IndicatorFactor : public distribution::factor::cnst::Factor {
    public:
        IndicatorFactor(categoric::VariablePtr var, std::size_t evidence);
    };
}

#endif
