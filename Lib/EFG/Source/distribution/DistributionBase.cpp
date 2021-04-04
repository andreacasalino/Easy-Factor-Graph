/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/DistributionBase.h>
#include <distribution/DistributionIterator.h>

namespace EFG::distribution {
    bool operator<(const Combination& a, const Combination& b) {
        for(std::size_t k=0; k<a.size(); ++k) {
            if(a.data()[k] != b.data()[k]) {
                return (a.data()[k] < b.data()[k]); 
            }
        }
        return false;
    };

    DistributionBase::DistributionBase(const categoric::Group& group, image::EvaluatorPtr evaluator) {
        this->group = std::make_unique<categoric::Group>(group);
        this->values = std::make_shared<std::map<Combination, float>>();
        this->evaluator = std::move(evaluator);
    }

    DistributionIterator DistributionBase::getIterator() const {
        return DistributionIterator(*this);
    }
}
