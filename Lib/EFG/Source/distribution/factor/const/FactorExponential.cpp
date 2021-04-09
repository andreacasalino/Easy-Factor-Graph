/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/factor/const/FactorExponential.h>
#include <distribution/factor/EvaluatorExponential.h>

namespace EFG::distribution::factor::cnst {
    FactorExponential::FactorExponential(const categoric::Group& group, float weight)
        : DistributionInstantiable(group, std::make_unique<EvaluatorExponential>(weight)) {
        // fill values inside domain
        throw 0;
    }

    FactorExponential::FactorExponential(const Factor& factor, float weight)
        : FactorExponential(factor.getGroup(), weight) {
        // fill values inside domain
        throw 0;
    }
    
}
