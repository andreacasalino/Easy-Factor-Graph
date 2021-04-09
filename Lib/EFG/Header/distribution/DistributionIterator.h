/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_DISTRIBUTION_ITERATOR_H
#define EFG_DISTRIBUTION_DISTRIBUTION_ITERATOR_H

#include <iterator/StlBidirectional.h>
#include <distribution/Distribution.h>

namespace EFG::distribution {
    class DistributionIterator 
        : public iterator::StlBidirectional<std::map<Combination, float>::const_iterator> {
    public:
        DistributionIterator(const Distribution& distribution);

        DistributionIterator(const DistributionIterator& ) = default;
        DistributionIterator& operator=(const DistributionIterator& ) = default;

        inline const Combination& getCombination() const { return this->cursor->first; }
        inline float getImage() const { return this->evaluator->evaluate(this->cursor->second); }

    private:
        std::shared_ptr<std::map<Combination, float>> values;
        EvaluatorPtr evaluator;
    };
}

#endif
