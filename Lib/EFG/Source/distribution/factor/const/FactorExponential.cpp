/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/factor/const/FactorExponential.h>
#include <distribution/factor/EvaluatorExponential.h>
#include <distribution/DistributionIterator.h>

namespace EFG::distribution::factor::cnst {
    FactorExponential::FactorExponential(const Factor& factor, float weight)
        : DistributionInstantiable(factor) {
        if (this->group->size() != this->values->size()) {
            this->emplaceEntireDomain();
        }
        this->evaluator = std::make_shared<EvaluatorExponential>(weight);
    }
    
    FactorExponential::FactorExponential(const FactorExponential& o) : DistributionInstantiable(o) {
        this->evaluator = std::make_shared<EvaluatorExponential>(o.getWeight());
    };

    float FactorExponential::getWeight() const {
        return static_cast<const EvaluatorExponential*>(this->evaluator.get())->getWeight();
    }
}
