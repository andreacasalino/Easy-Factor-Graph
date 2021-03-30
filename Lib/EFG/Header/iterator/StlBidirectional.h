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
    template<typename IteratorStl>
    // check inherits from std::Iterator
    class StlBidirectional : public Forward {
    public:
        StlBidirectional(const IteratorStl& begin, const IteratorStl& end) 
            : cursor(begin)
            , end(end) {
        };

        StlBidirectional(const StlBidirectional& ) = default;
        StlBidirectional& operator=(const StlBidirectional& ) = default;

        void operator++() final {
            return ++this->cursor;
        };

        void operator--() final {
            return --this->cursor;
        };

        bool operator==(std::nullptr_t) const final {
            return (this->cursor == this->end);
        };

    private:
        IteratorStl cursor;
        const IteratorStl end;
    };
}

#endif
