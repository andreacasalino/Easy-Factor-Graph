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
    /**
     * @brief An object able to iterate the domain of a distribution
     */
    class DistributionIterator 
        : public iterator::StlBidirectional<std::map<categoric::Combination, float>::const_iterator> {
    public:
        /**
         * @param the distribution to iterate
         */
        explicit DistributionIterator(const Distribution& distribution);

        DistributionIterator(const DistributionIterator& ) = default;
        DistributionIterator& operator=(const DistributionIterator& ) = default;

        /**
         * @return the combination currently pointed by the iterator
         */
        inline const categoric::Combination& getCombination() const { return this->cursor->first; }

        /**
         * @return the image of the combination currently pointed by the iterator
         */
        inline float getImage() const { return this->evaluator->evaluate(this->cursor->second); }

        /**
         * @return the row image of the combination currently pointed by the iterator
         */
        inline float getImageRaw() const { return this->cursor->second; }

        /**
         * @return the number of explicitly combinations placed inside the distribution this iterator was built from
         */
        inline std::size_t getNumberOfValues() const { return this->values->size(); }

    private:
        std::shared_ptr<std::map<categoric::Combination, float>> values;
        EvaluatorPtr evaluator;
    };
}

#endif
