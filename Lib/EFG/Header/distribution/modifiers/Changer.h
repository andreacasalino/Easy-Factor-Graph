/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_CHANGER_H
#define EFG_DISTRIBUTION_CHANGER_H

#include <distribution/Distribution.h>

namespace EFG::distribution {
    class Changer
        : public virtual Distribution {
    public:
        std::map<Combination, float>::const_iterator add(const Combination& comb, const float& value);

        // put 0 for all not added combinations
        void emplaceEntireDomain();

        void setImageEntireDomain(const float& value);

        inline void clear() { this->values->clear(); };
    };
}

#endif
