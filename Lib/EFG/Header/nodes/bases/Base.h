/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_BASE_H
#define EFG_NODES_BASE_H

namespace EFG::nodes {
    class Base {
    public:
        virtual ~Base() = default;

        Base(const Base&) = delete;
        Base& operator=(const Base&) = delete;

    protected:
        Base() = default;
    };
}

#endif
