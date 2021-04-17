/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_SETTER_H
#define EFG_DISTRIBUTION_SETTER_H

#include <distribution/Distribution.h>

namespace EFG::distribution {
    class Setter
        : public virtual Distribution {
    public:
        inline void replaceGroup(const categoric::Group& newGroup) { *this->group = newGroup; };

        void set(const Combination& comb, const float& value);
    };
}

#endif
