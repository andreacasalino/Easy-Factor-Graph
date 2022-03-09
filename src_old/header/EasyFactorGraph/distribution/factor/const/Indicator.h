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
    /**
     * @brief An indicator distirbution having only one combination explicitly stated,
     * whose image is equal to 1
     */
    class IndicatorFactor : public distribution::factor::cnst::Factor {
    public:
        /**
         * @param the variable this indicator function must refer to
         * @param the only combination to consider for the indicator distribution
         * @throw when evidence is inconsistent for the passed variable
         */
        IndicatorFactor(categoric::VariablePtr var, std::size_t evidence);
    };
}

#endif
