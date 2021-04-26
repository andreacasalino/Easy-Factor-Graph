/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_CATEGORIC_RANGE_H
#define EFG_CATEGORIC_RANGE_H

#include <categoric/Group.h>
#include <iterator/Forward.h>
#include <categoric/Combination.h>
#include <vector>

namespace EFG::categoric {
    /**
     * @brief This object allows you to iterate all the elements in the joint domain of a group of variables, without precomputing all the elements in such domain.
     * For example when having a domain made by variables = { A (size = 2), B (size = 3), C (size  = 2)  }, the elements in the
     * joint domain that will be iterated are:
     * <0,0,0>
     * <0,0,1>
     * <0,1,0>
     * <0,1,1>
     * <0,2,0>
     * <0,2,1>
     * <1,0,0>
     * <1,0,1>
     * <1,1,0>
     * <1,1,1>
     * <1,2,0>
     * <1,2,1>
     * After construction, the Range object starts to point to the first element in the joint domain <0,0,...>. Then, when incrementing the object,
     * the following element is pointed. When calling get() the current pointed element can be accessed.
     */
    class Range : public iterator::Forward {
    public:
        /** @param the group of variables whose joint domain must be iterated 
         */
        explicit Range(const std::set<VariablePtr>& group);

        Range(const Range& ) = default;
        Range& operator=(const Range& ) = default;

        /** 
         * @return the current pointed combination
         */
        inline const Combination& get() const { return this->combination; };

        /** 
         * @brief Make the object to point to the next element in the joint domain.
         * @throw if the current pointed element is the last one.
         */
        void operator++() final;

        inline bool operator==(std::nullptr_t) const final { return this->isAtEnd; };

        /** 
         * @brief Make the object to point to the first element of the joint domain <0,0,...>, i.e.
         * reset the status as it is after construction.
         */
        void reset();

    private:
        std::vector<size_t>		sizes;
        Combination             combination;
        bool                    isAtEnd = false;
    };
}

#endif
