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
        /** 
         * @param the combiantion to add
         * @param row image value to consider
         * @return an iterator pointing to the newly added combination
         */
        std::map<categoric::Combination, float>::const_iterator add(const categoric::Combination& comb, const float& value);

        /**
         * @brief fill all not explicitly stated combinations with zeros
         */
        void emplaceEntireDomain();

        /**
         * @brief sets the raw images of all the combinations equal to the passed value
         */
        void setImageEntireDomain(const float& value);

        /**
         * @brief remove all the combinations from the distribution
         */
        inline void clear() { this->values->clear(); };
    };
}

#endif
