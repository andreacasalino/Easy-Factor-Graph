/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_MODIFIABLE_H
#define EFG_DISTRIBUTION_MODIFIABLE_H

#include <distribution/Distribution.h>
#include <observers/MultiObservable.h>

namespace EFG::distribution {
    class DistributionModifiable
        : public virtual Distribution
        , public observer::MultiObservable {
    protected:
        void checkCombination(const Combination& comb, const float& value) const;

        void checkModifiable() const;
    };
}

#endif
