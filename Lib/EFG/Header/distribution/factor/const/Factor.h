/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_FACTOR_CONST_FACTOR_H
#define EFG_DISTRIBUTION_FACTOR_CONST_FACTOR_H

#include <distribution/DistributionInstantiable.h>

namespace EFG::distribution::factor::cnst {
    /**
     * @brief A factor using the EvaluatorBasic object to convert the row images into images
     */
    class Factor : public DistributionInstantiable {
    public:
        /**
         * @brief Builds a simple correlating or anticorrelating factor.
         * @param the variables representing the domain of this distribution
         * @param when passing:
         *  - true, a simple correlating potential is built. Such a distribution
         *    has the images equal to 1 only for those combinations for which the variables have all the same values (<1,1,1>, <2,2,2>, <0,0>, etc...) and 0 for all the others
         *  - false, a simple anticorrelating potential is built. Such a distribution
         *    has the images equal to 0 for those combinations for which the variables have all the same values (<1,1,1>, <2,2,2>, <0,0>, etc...) and 1 for all the others 
         */
        Factor(const std::set<categoric::VariablePtr>& group, bool corrOrAnti);

        Factor(const Factor& o) : DistributionInstantiable(o) {};
        Factor(Factor&& o) : DistributionInstantiable(std::move(o)) {};

        /**
         * @brief Copies all the images (not row) of the passed distribution in order to build a generic Factor
         */
        explicit Factor(const Distribution& o);

        /**
         * @brief Merges all the passed distribution into a single Factor.
         * The domain of the Factor is obtained merging the domains of the distributions,
         * while the image are obtained multiplying the images of the passed distribution.
         */
        template<typename ... Distributions>
        Factor(const Distribution* first, const Distribution* second, Distributions ... distr)
            : Factor(pack(first, second, distr...)) {
        };

        /**
         * @brief Merges all the passed distribution into a single Factor.
         * The domain of the Factor is obtained merging the domains of the distributions,
         * while the image are obtained multiplying the images of the passed distribution.
         */
        explicit Factor(const std::set<const Distribution*>& distr);

        /**
         * @brief Builds the factor by taking all the combinations of the passed distribution matching with
         * the passed combination.
         * Suppose toMarginalize has a domain of variables equal to <A,B,C,D> and the passed comb is <0,1> and
         * evidences is <B,C>. The built factor will have a domain of varaibles equal to <A,D>, with the combinations-images
         * of toMarginalize (taking only the part referring to A,D) that have B=0 and C=1.
         */
        Factor(const Distribution& toMarginalize, const Combination& comb, const categoric::Group& evidences);

        /**
         * @brief Build the Factor by importing the information from the file
         * @param the group of variables to assume
         * @param the location of a file storing the combinations and the row images in a matrix of numbers:
         * each row has the combination value and at the end the row image
         */
        Factor(const std::set<categoric::VariablePtr>& group, const std::string& fileName);

    protected:
        Factor(const std::set<categoric::VariablePtr>& group);

        template<typename ... Distributions>
        static std::set<const Distribution*> pack(const Distribution* first, const Distribution* second, Distributions ... distr) {
            std::set<const Distribution*> packed;
            pack(packed , first, second, distr...);
            return packed;
        };
        template<typename ... Distributions>
        static void pack(std::set<const Distribution*>& packed, const Distribution* first, Distributions ... distr) {
            packed.emplace(first);
            pack(packed, distr...);
        }
        static void pack(std::set<const Distribution*>& packed, const Distribution* first) {
            packed.emplace(first);
        }
    };
}

#endif
