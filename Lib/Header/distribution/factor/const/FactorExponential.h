/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_FACTOR_CONST_FACTOR_EXP_H
#define EFG_DISTRIBUTION_FACTOR_CONST_FACTOR_EXP_H

#include <distribution/factor/const/Factor.h>
#include <distribution/DistributionSetter.h>

namespace EFG::distribution::factor::cnst {
    /**
     * @brief A factor using the EvaluatorExponential object to convert the raw images into images
     */
    class FactorExponential
        : public DistributionInstantiable
        , virtual protected DistributionSetter {
    public:
        /**
         * @param the factor whose raw images are copied
         * @param the weight to pass to the EvaluatorExponential
         */
        FactorExponential(const Factor& factor, float weight);

        FactorExponential(const FactorExponential& o);

        /**
         * @return the weight of the EvaluatorExponential
         */
        float getWeight() const;
    };
}

#endif
