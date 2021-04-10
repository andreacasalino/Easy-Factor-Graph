/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_FACTOR_MODIFIABLE_FACTOR_EXP_H
#define EFG_DISTRIBUTION_FACTOR_MODIFIABLE_FACTOR_EXP_H

#include <distribution/factor/const/FactorExponential.h>
#include <distribution/modifiers/Changer.h>
#include <distribution/modifiers/Setter.h>

namespace EFG::distribution::factor::modif {
    class FactorExponential
        : public cnst::FactorExponential
        , public Setter {
    public:
        FactorExponential(const FactorExponential& factor, float weight) : cnst::FactorExponential(factor) {};

        FactorExponential(const FactorExponential& o) : cnst::FactorExponential(o) {};

        inline FactorExponential& operator=(const FactorExponential& o) { this->DistributionInstantiable::operator=(o); return *this; };

        void setWeight(float w);
    };
}

#endif
