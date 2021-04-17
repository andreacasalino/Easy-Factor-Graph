/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_FACTOR_CONST_FACTOR_EXP_H
#define EFG_DISTRIBUTION_FACTOR_CONST_FACTOR_EXP_H

#include <distribution/factor/const/Factor.h>
#include <distribution/modifiers/Changer.h>

namespace EFG::distribution::factor::cnst {
    class FactorExponential 
        : public DistributionInstantiable
        , protected Changer {
    public:
        FactorExponential(const Factor& factor, float weight);

        FactorExponential(const FactorExponential& o);

        float getWeight() const;
    };
}

#endif
