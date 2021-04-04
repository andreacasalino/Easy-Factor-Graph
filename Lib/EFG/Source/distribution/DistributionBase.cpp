/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/DistributionBase.h>
#include <distribution/DistributionIterator.h>

namespace EFG::distribution {
    DistributionBase::DistributionBase(const categoric::Group& group, image::EvaluatorPtr evaluator) {
        this->group = std::make_unique<categoric::Group>(group);
        this->values = std::make_shared<std::map<Combination, float>>();
        this->evaluator = std::move(evaluator);
    }

    DistributionIterator DistributionBase::getIterator() const {
        return DistributionIterator(*this);
    }
}
