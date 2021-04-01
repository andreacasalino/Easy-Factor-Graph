/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_EXPONENTIAL_DISTRIBUTION_H
#define EFG_DISTRIBUTION_EXPONENTIAL_DISTRIBUTION_H

#include <distribution/Distribution.h>

namespace EFG::distribution {
    class ExponentialDistribution : public Distribution {
    public:
        ExponentialDistribution(const categoric::Group& variables, const float& weight = 1.f);

        float getWeight() const;
        void setWeight(float w);
    };
}

#endif
