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

        /**
         * @brief sets the image of the poassed combiantion.
         * In case the combination is currently not part of the distribution,
         * it is added.
         * @param the combination whose image must be set
         * @param the image value to assume
         */
        void set(const Combination& comb, const float& value);
    };
}

#endif
