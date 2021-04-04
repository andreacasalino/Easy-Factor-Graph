/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_FACTOR_CONST_H
#define EFG_DISTRIBUTION_FACTOR_CONST_H

#include <distribution/DistributionBase.h>

namespace EFG::distribution {
    class FactorConst : public DistributionBase {
    public:
        FactorConst(const categoric::Group& group);

        // FactorConst(const Distribution& o);
        // FactorConst& operator=(const Distribution& o);

        // FactorConst(Distribution&& o);
        // FactorConst& operator=(Distribution&& o);

        template<typename ... Distributions>
        FactorConst(const Distribution* first, const Distribution* second, Distributions ... distr)
            : FactorConst(pack(first, second, distr...)) {
        };

        // basic evaluator is assumed (even for exponential distribution)
        FactorConst(const std::set<const Distribution*>& distr);

        // marginalization
        FactorConst(const DistributionBase& toMarginalize, const Combination& comb, const categoric::Group& evidences);

    protected:
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
