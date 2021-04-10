/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/factor/modifiable/FactorExponential.h>
#include <distribution/factor/EvaluatorExponential.h>

namespace EFG::distribution::factor::modif {
    void FactorExponential::setWeight(float w) {
        static_cast<EvaluatorExponential*>(this->evaluator.get())->setWeight(w);
    }
}
