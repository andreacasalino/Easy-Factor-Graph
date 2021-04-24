/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_ITERATOR_STL_BIDIRECTIONAL_H
#define EFG_ITERATOR_STL_BIDIRECTIONAL_H

#include <iterator/Bidirectional.h>

namespace EFG::iterator {
    /** @brief A bidirectional iterator built on top of an std iterator type
     */
    template<typename IteratorStl>
    class StlBidirectional : public Bidirectional {
    public:
        StlBidirectional(const IteratorStl& begin, const IteratorStl& end) 
            : cursor(begin)
            , end(end) {
        };

        StlBidirectional(const StlBidirectional& ) = default;
        StlBidirectional& operator=(const StlBidirectional& ) = default;

        inline void operator++() final {
            ++this->cursor;
        };

        inline void operator--() final {
            --this->cursor;
        };

        inline bool operator==(std::nullptr_t) const final {
            return (this->cursor == this->end);
        };

    protected:
        IteratorStl cursor;
        const IteratorStl end;
    };
}

#endif
