/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_CATEGORIC_GROUP_H
#define EFG_CATEGORIC_GROUP_H

#include <categoric/Variable.h>
#include <set>

namespace EFG::categoric {
    class Group {
    public:
        Group(VariablePtr varA, VariablePtr varB);

        template<typename ... Vars>
        Group(VariablePtr varA, VariablePtr varB, Vars ... vars) 
            :Group(varA, varB) {
            this->add(vars...);
        }

        // throw in case a variable with same name is already part of group
        void add(VariablePtr var);

        /** \brief Returns the size of the joint domain
        */
        std::size_t size() const;

        inline const std::set<VariablePtr>& getGroup() const { return this->group; }
        
    protected:
        template<typename ... Vars>
        void add(VariablePtr var, Vars ... vars) {
            this->add(var);
            this->add(vars...);
        }
        inline void add() { return; }

    // data
        std::set<VariablePtr> group;
    };
}

#endif
