/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_FACTOR_CONST_FACTOR_EXP_H
#define EFG_DISTRIBUTION_FACTOR_CONST_FACTOR_EXP_H

#include <distribution/factor/const/Factor.h>

namespace EFG::distribution::factor::cnst {
    class FactorExponential : public DistributionInstantiable {
    public:
        FactorExponential(const categoric::Group& group, float weight);

        FactorExponential(const Factor& factor, float weight);

        // FactorExponentialConst(const FactorExponentialConst& o);
        // FactorExponentialConst& operator=(const FactorExponentialConst& o);
    };
}

#endif
