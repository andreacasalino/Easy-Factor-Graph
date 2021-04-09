/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_FACTOR_MODIFIABLE_FACTOR_H
#define EFG_DISTRIBUTION_FACTOR_MODIFIABLE_FACTOR_H

#include <distribution/factor/const/Factor.h>
#include <distribution/modifiers/Changer.h>
#include <distribution/modifiers/Setter.h>

namespace EFG::distribution::factor::modif {
    class Factor 
        : public cnst::Factor
        , public Changer
        , public Setter {
    public:
        template<typename ... Args>
        Factor(Args&&... args)
            : cnst::Factor(std::forward<Args>(args)...) {
        };

        Factor(const categoric::Group& group)
            : cnst::Factor(group) {
        };
    };
}

#endif
