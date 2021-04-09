/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_SETTER_H
#define EFG_DISTRIBUTION_SETTER_H

#include <distribution/modifiers/DistributionModifiable.h>

namespace EFG::distribution {
    class Setter
        : public virtual DistributionModifiable {
    public:
        void replaceGroup(const categoric::Group& newGroup);

        void set(const Combination& comb, const float& value);
    };
}

#endif
