/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_MUTABLE_H
#define EFG_DISTRIBUTION_MUTABLE_H

#include <distribution/Distribution.h>
#include <observers/MultiObservable.h>

namespace EFG::distribution {
    class DistributionSettable
        : public virtual Distribution
        , public observer::MultiObservable {
    public:
        void replaceGroup(const categoric::Group& newGroup);

        void set(const Combination& comb, const float& value);

    protected:
        void checkCombination(const Combination& comb, const float& value) const;

        void checkModifiable() const;
    };

    class DistributionModifiable : public DistributionSettable {
    public:
        void clear();

        std::map<Combination, float>::const_iterator add(const Combination& comb, const float& value);

        // put 0 for all not added combinations
        void emplaceEntireDomain();

        void setImageEntireDomain(const float& value);
    };
}

#endif
