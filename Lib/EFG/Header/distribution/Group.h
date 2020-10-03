/**
 * Author:    Andrea Casalino
 * Created:   05.03.2019
*
* report any bug to andrecasa91@gmail.com.
**/

#pragma once
#ifndef __EFG_DISTRIBUTION_GROUP_H__
#define __EFG_DISTRIBUTION_GROUP_H__

#include <CategoricVariable.h>
#include <util/ObservedSubject.h>

namespace EFG::distr {

    class Group {
    public:
        inline const std::vector<CategoricVariable*>& GetVariables() const { return this->group; };

        /** \brief Use this method for replacing the set of variables in the domain of this distribution.
        * Variables in new_var must be equal in number to the original set of variables and
        * must have the same sizes.
        *
        * @param[in] new_var variables to consider for the substitution
        */
        void replace(const std::vector<CategoricVariable*>& newGroup);

        virtual ~Group() {};

        Group(const Group&) = delete;
        void operator=(const Group&) = delete;
    protected:
        Group(const std::vector<CategoricVariable*>& group);
    private:
        // data
        std::vector<CategoricVariable*>      group;
        std::vector<sbj::Subject::Observer>  groupObservers;
    };

}

#endif