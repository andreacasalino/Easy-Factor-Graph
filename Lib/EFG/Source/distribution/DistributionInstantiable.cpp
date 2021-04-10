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

    DistributionInstantiable& DistributionInstantiable::operator=(const DistributionInstantiable& o) {
        *this->group = *o.group;
        *this->values = *o.values;
        return *this;
    }

    DistributionInstantiable::DistributionInstantiable(const DistributionInstantiable& o) {
        this->group = std::make_unique<categoric::Group>(o.group);
        this->values = std::make_shared<std::map<Combination, float>>(o.values);
        this->evaluator = o.evaluator->copy();
    }

    DistributionInstantiable& DistributionInstantiable::operator=(DistributionInstantiable&& o) {
        *this->group = *o.group;
        this->values = o.values;
        o.values = std::make_shared<std::map<Combination, float>>();
        return *this;
    }

    DistributionInstantiable::DistributionInstantiable(DistributionInstantiable&& o) {
        this->group = std::make_unique<categoric::Group>(o.group);
        this->values = o.values;
        o.values = std::make_shared<std::map<Combination, float>>();
        this->evaluator = o.evaluator->copy();
    }
}
