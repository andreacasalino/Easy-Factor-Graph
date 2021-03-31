// ExponentialDistribution(const categoric::Group& variables, const float& weight = 1.f)


/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/ExponentialDistribution.h>
#include <distribution/image/Exponential.h>

namespace EFG::distribution {
    ExponentialDistribution::ExponentialDistribution(const categoric::Group& variables, const float& weight)
        : Distribution(std::make_shared<image::Exponential>(weight) , variables) {
    }
}
