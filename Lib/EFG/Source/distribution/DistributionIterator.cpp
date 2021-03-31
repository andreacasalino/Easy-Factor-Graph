/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/DistributionIterator.h>

namespace EFG::distribution {
    DistributionIterator::DistributionIterator(const Distribution& distribution)
        : iterator::StlBidirectional<std::map<Combination, float>::const_iterator>(
            distribution.values.begin(),
            distribution.values.end()
        ) {
        this->evaluator = distribution.evaluator;
    }

    DistributionIterator Distribution::getIterator() const {
        return DistributionIterator(*this);
    }
}
