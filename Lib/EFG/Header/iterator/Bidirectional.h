/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_ITERATOR_BIDIRECTIONAL_H
#define EFG_ITERATOR_BIDIRECTIONAL_H

#include <iterator/Forward.h>

namespace EFG::iterator {
    class Bidirectional : public Forward{
    public:
        virtual void operator--() = 0;
    };
}

#endif
