/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_FACTOR_MODIFIABLE_FACTOR_EXP_H
#define EFG_DISTRIBUTION_FACTOR_MODIFIABLE_FACTOR_EXP_H

#include <distribution/factor/const/FactorExponential.h>

namespace EFG::distribution::factor::modif {
    class FactorExponential
        : public cnst::FactorExponential
        , virtual public DistributionSetter {
    public:
        FactorExponential(const cnst::Factor& factor, float weight) : cnst::FactorExponential(factor, weight) {};

        explicit FactorExponential(const cnst::FactorExponential& o) : cnst::FactorExponential(o) {};

        FactorExponential(const FactorExponential& o) : FactorExponential(static_cast<const cnst::FactorExponential&>(o)) {};

        inline FactorExponential& operator=(const FactorExponential& o) { this->DistributionInstantiable::operator=(o); return *this; };

        /**
         * @brief sets the weight used by teh exponential function converting the raw images
         */
        void setWeight(float w);
    };
}

#endif
