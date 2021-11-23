/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_FACTOR_MODIFIABLE_FACTOR_H
#define EFG_DISTRIBUTION_FACTOR_MODIFIABLE_FACTOR_H

#include <distribution/factor/const/Factor.h>
#include <distribution/DistributionSetter.h>

namespace EFG::distribution::factor::modif {
    class Factor 
        : public cnst::Factor
        , public DistributionSetter {
    public:
        template<typename ... Args>
        Factor(Args&&... args)
            : cnst::Factor(std::forward<Args>(args)...) {
        };

        explicit Factor(const std::set<categoric::VariablePtr>& group)
            : cnst::Factor(group) {
        };

        Factor(const Factor& o) : cnst::Factor(o) {};
        Factor(Factor&& o) : cnst::Factor(std::move(o)) {};

        inline Factor& operator=(const Factor& o) { this->DistributionInstantiable::operator=(o); return *this; };
        inline Factor& operator=(Factor&& o) { this->DistributionInstantiable::operator=(std::move(o)); return *this; };

        /**
         * @brief sets all raw images equal to 0 and deallocate all combinations
         */
        inline void clear() { this->values->clear(); };
    };
}

#endif
