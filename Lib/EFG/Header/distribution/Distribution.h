/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_DISTRIBUTION_H
#define EFG_DISTRIBUTION_DISTRIBUTION_H

#include <categoric/Group.h>
#include <distribution/Evaluator.h>
#include <categoric/Combination.h>
#include <map>

namespace EFG::distribution {
    class DistributionIterator;
    class DistributionFinder;

    /** 
     * @brief Base object for any kind of categoric distribution.
     * Any kind of categoric distribution has:
     *  - A domain, represented by the combinations in the joint domain of the Group associated to this distribution
     *  - Raw images set, which are positive values associated to each element in the domain
     *  - Images set, which are the image values associated to each element in the domain. They can be obtained by applying a certain function f(x) to the raw images
     * In order to save memory, the combinations having an image equal to 0 are not explicitly saved even if they are accounted for the opreations involving this distribution.
     */
    class Distribution {
    friend class DistributionIterator;
    friend class DistributionFinder;
    public:
        virtual ~Distribution() = default;

        inline const categoric::Group& getGroup() const { return *this->group; };

        /**
         * @return a DistributionIterator referring to this object
         */
        DistributionIterator getIterator() const;

        /** 
         * @brief searches for the image associated to an element in the domain
         * @return the value of the image. 
         */
        float find(const categoric::Combination& comb) const;

        /**
         * @brief searches for the raw image associated to an element in the domain
         * @return the value of the raw image.
         */
        float findRaw(const categoric::Combination& comb) const;

        /**
         * @return a DistributionFinder referring to this object
         */
        DistributionFinder getFinder(const std::set<categoric::VariablePtr>& containingGroup) const;

        /**
         * @return the probabilities associated to each combination in the domain, when assuming only the existance of this distribution. 
         * Such probabilities are the normalized images.
         * The order of returned values, refer to the combination order obtained by iterating with the categoric::Range object.
         */
        std::vector<float> getProbabilities() const;

    protected:
        Distribution() = default;

        void checkCombination(const categoric::Combination& comb, const float& value) const;

        std::unique_ptr<categoric::Group> group;
        /**
         * @brief the ordered pairs of <domain combination, raw image value>
         */
        std::shared_ptr<std::map<categoric::Combination, float>> values;
        /**
         * @brief the function used to convert raw images into images
         */
        EvaluatorPtr evaluator;
    };

    typedef std::shared_ptr<Distribution> DistributionPtr;
}

#endif
