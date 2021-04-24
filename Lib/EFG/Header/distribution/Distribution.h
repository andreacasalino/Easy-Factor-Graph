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
     *  - A domain of combinations, represented by the joint domain of a categoric Group
     *  - An set of row image values associated to each element in the domain of the distribution
     *  - An set of iamges associated to each element in the domain, obtained by applying a certain operation to the row images
     * In order to save memory, it is possible that not all the image values are explicitly saved 
     * and for the corresponding combinations the images are assumed equal to 0.
     */
    class Distribution {
    friend class DistributionIterator;
    friend class DistributionFinder;
    public:
        virtual ~Distribution() = default;

        inline const categoric::Group& getGroup() const { return *this->group; };

        /**
         * @return a DistributionIterator referring to thís object
         */
        DistributionIterator getIterator() const;

        /** 
         * @brief searches for the image associated to an element in the domain
         * @return the value of the image. 
         */
        float find(const categoric::Combination& comb) const;

        /**
         * @brief searches for the image associated to an element in the domain
         * @return the value of the image.
         */
        float findRaw(const categoric::Combination& comb) const;

        /**
         * @return a DistributionFinder referring to thís object
         */
        DistributionFinder getFinder(const std::set<categoric::VariablePtr>& containingGroup) const;

        /**
         * @return the probabilities associated to each combination (also the ones for which the image is not explicitly reported) 
         * in the domain of this potential, when assuming only the existance of this distribution. Such probabilities are the normalized images.
         * The order of the corresponding combinations can be also accessed using a Range object built with the variables in the group of this distribution
         */
        std::vector<float> getProbabilities() const;

    protected:
        Distribution() = default;

        void checkCombination(const categoric::Combination& comb, const float& value) const;

        std::unique_ptr<categoric::Group> group;
        /**
         * @brief the ordered pairs of <combination , row image>
         */
        std::shared_ptr<std::map<categoric::Combination, float>> values;
        /**
         * @brief the function used to convert row images to images
         */
        EvaluatorPtr evaluator;
    };

    typedef std::shared_ptr<Distribution> DistributionPtr;
}

#endif
