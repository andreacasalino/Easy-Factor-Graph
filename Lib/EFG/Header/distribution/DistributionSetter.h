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
    class DistributionSetter : public virtual Distribution {
    public:
        /**
         * @brief replace the variables describing the domain of this distribution
         */
        inline void replaceGroup(const categoric::Group& newGroup) { *this->group = newGroup; };

        /**
         * @brief sets the image of the passed combination.
         * In case the combination is currently not part of the distribution, it is added with the passe raw image value.
         * @param the combination whose raw image must be set
         * @param the raw image value to assume
         */
        void setImageRaw(const categoric::Combination& comb, const float& value);

        /**
         * @brief creates all the non explicitly set combinations and assumed for them a 0 raw image value.
         */
        void fillDomain();

        /**
         * @brief sets the raw images of all the combinations equal to the passed value
         */
        void setAllImagesRaw(const float& value);
    };
}

#endif
