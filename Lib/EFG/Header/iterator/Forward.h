/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_ITERATOR_FORWARD_H
#define EFG_ITERATOR_FORWARD_H

#include <cstddef> // std::nullptr_t

namespace EFG::iterator {
    class Forward {
    public:
        virtual void operator++() = 0;

        // nullptr when the iterator is at the end, i.e. not anymore incrementable
        virtual bool operator==(std::nullptr_t) const = 0;

        inline bool operator!=(std::nullptr_t) const {
            return !(*this == nullptr);
        };
    };

    template<typename Iter, typename Action>
    void forEach(Iter& iter, Action action) {
        while (iter != nullptr) {
            action(iter);
            ++iter;
        }
    };

    template<typename Iter, typename ActionCondition>
    void forEachConditioned(Iter& iter, ActionCondition action) {
        while (iter != nullptr) {
            if(!action(iter)) {
                break;
            }
            ++iter;
        }
    };
}

#endif
