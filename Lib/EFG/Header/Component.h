/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_COMPONENT_H
#define EFG_COMPONENT_H

namespace EFG {
    class Component {
    public:
        virtual ~Component() = default;

        Component(const Component&) = delete;
        Component& operator=(const Component&) = delete;

    protected:
        Component() = default;
    };
}

#endif
