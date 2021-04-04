/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/FactorExponentialConst.h>
#include <distribution/image/Exponential.h>

namespace EFG::distribution {
    FactorExponentialConst::FactorExponentialConst(const categoric::Group& group, float weight)
        : DistributionBase(group, std::make_unique<image::Exponential>(weight)) {
        // fill values inside domain
        throw 0;
    }

    FactorExponentialConst::FactorExponentialConst(const FactorConst& factor, float weight) 
        : FactorExponentialConst(factor.getGroup(), weight) {
        // fill values inside domain
        throw 0;
    }
    
}
