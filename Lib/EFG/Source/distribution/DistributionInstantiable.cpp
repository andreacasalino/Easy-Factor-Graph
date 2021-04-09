/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/DistributionInstantiable.h>

namespace EFG::distribution {
    DistributionInstantiable::DistributionInstantiable(const categoric::Group& group, EvaluatorPtr evaluator) {
        this->group = std::make_unique<categoric::Group>(group);
        this->values = std::make_shared<std::map<Combination, float>>();
        this->evaluator = std::move(evaluator);
    }
}
