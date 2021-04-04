/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_BASE_H
#define EFG_DISTRIBUTION_BASE_H

#include <distribution/Distribution.h>

namespace EFG::distribution {
    class DistributionBase : virtual public distribution::Distribution {
    friend class DistributionIterator;
    public:
        DistributionIterator getIterator() const final;

    protected:
        DistributionBase(const categoric::Group& group, image::EvaluatorPtr evaluator);

        inline categoric::Group* getGroupPtr() const final { return this->group.get(); };
        inline std::map<Combination, float>* getValuesPtr() const final { return this->values.get(); };
        inline image::Evaluator* getEvaluatorPtr() const final { return this->evaluator.get(); };

        std::unique_ptr<categoric::Group> group;
        std::shared_ptr<std::map<Combination, float>> values;
        image::EvaluatorPtr evaluator;

    };
}

#endif
