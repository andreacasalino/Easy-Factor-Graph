/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_FACTOR_MODIFIABLE_FACTOR_EXP_H
#define EFG_DISTRIBUTION_FACTOR_MODIFIABLE_FACTOR_EXP_H

#include <distribution/factor/const/FactorExponential.h>
#include <distribution/modifiers/Changer.h>
#include <distribution/modifiers/Setter.h>

namespace EFG::distribution::factor::modif {
    class FactorExponential
        : public cnst::FactorExponential
        , public Setter {
    public:
        template<typename ... Args>
        FactorExponential(Args&&... args)
            : FactorExponential(std::forward<Args>(args)...) {
        };
    };
}

#endif
/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_FACTOR_EXP_CONST_H
#define EFG_DISTRIBUTION_FACTOR_EXP_CONST_H

#include <distribution/DistributionBase.h>
#include <distribution/FactorConst.h>

namespace EFG::distribution {
    class FactorExponentialConst : public distribution::DistributionBase {
    public:
        FactorExponentialConst(const categoric::Group& group, float weight);

        FactorExponentialConst(const FactorConst& factor, float weight);

        // FactorExponentialConst(const FactorExponentialConst& o);
        // FactorExponentialConst& operator=(const FactorExponentialConst& o);
    };
}

#endif
