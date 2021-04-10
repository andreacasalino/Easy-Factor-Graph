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
    class Factor : public DistributionInstantiable {
    public:
        Factor(const categoric::Group& group, bool corrOrAnti);

        Factor(const Factor& o) : DistributionInstantiable(o) {};
        Factor(Factor&& o) : DistributionInstantiable(std::move(o)) {};

        template<typename ... Distributions>
        Factor(const Distribution* first, const Distribution* second, Distributions ... distr)
            : Factor(pack(first, second, distr...)) {
        };

        Factor(const std::set<const Distribution*>& distr);

        // marginalization
        Factor(const Distribution& toMarginalize, const Combination& comb, const categoric::Group& evidences);

    protected:
        Factor(const categoric::Group& group);

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
