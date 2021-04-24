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
    inline bool operator<(const VariablePtr& a, const VariablePtr& b) {
        return (a->name() < b->name());
    };

    inline bool operator==(const VariablePtr& a, const VariablePtr& b) {
        return *a == *b;
    };

    /**
     * @brief An ensemble of categoric variables. Each variable in the ensemble 
     * should have its own unique name.
     */
    class Group {
    public:
        /**
         * @param the initial variables of the group
         */
        explicit Group(const std::set<VariablePtr>& group);

        /**
         * @param the initial variable to put in the group
         */
        explicit Group(VariablePtr var);

        /**
         * @param the first initial variable to put in the group
         * @param the second initial variable to put in the group
         * @throw when the 2 variables have the same names
         */
        Group(VariablePtr varA, VariablePtr varB);

        /**
         * @param the first initial variable to put in the group
         * @param the second initial variable to put in the group
         * @param all the other initial varaibles
         */
        template<typename ... Vars>
        Group(VariablePtr varA, VariablePtr varB, Vars ... vars) {
            this->add(varA);
            this->add(varB, vars...);
        }

        explicit Group(const Group& ) = default;

        /**
         * @throw In case of size mismatch with the previous variables set:
         * the sizes of the 2 groups should be the same and the elements in
         * the same positions must have the same domain size
         */
        Group& operator=(const Group& o);

        /**
         * @brief the addresses of the wrapped VariablePtr pointers are compared
         */
        inline bool operator==(const Group& o) const { return (this->group == o.group); };

        /**
         * @param the variable to add to the group
         * @throw in case a variable with the same name is already part of the group
         */
        void add(VariablePtr var);

        /**
         * @brief sets the new group of variables this group refers to.
         * @throw In case of size mismatch with the previous variables set:
         * the sizes of the 2 groups should be the same and the elements in 
         * the same positions must have the same domain size
         */
        void replace(const std::set<VariablePtr>& newGroup) { *this = Group(newGroup); };

        /**
         * @brief sets the new group of variables this group refers to.
         * @throw In case of size mismatch with the previous variables set:
         * the sizes of the 2 groups should be the same and the elements in
         * the same positions must have the same domain size
         */
        template<typename ... Vars>
        void replace(VariablePtr varA, VariablePtr varB, Vars ... vars) { *this = Group(varA, varB, vars...); };

        /** @return the size of the joint domain of the group.
         * For example the group <A,B,C> with sizes <2,4,3> will have a joint domain of size 2X4X3 = 24
         */
        std::size_t size() const;

        inline const std::set<VariablePtr>& getVariables() const { return this->group; }
        
    protected:
        template<typename ... Vars>
        void add(VariablePtr var, Vars ... vars) {
            this->add(var);
            this->add(vars...);
        }

        std::set<VariablePtr> group;
    };

    /** @brief get the complementary group of the subset, w.r.t a certain set.
     * For instance the complementary of subset <A,C> w.r.t. <A,B,C,D,E> is <B,D,E>
     */
    std::set<VariablePtr> getComplementary(const std::set<VariablePtr>& set, const std::set<VariablePtr>& subset);
}

#endif
