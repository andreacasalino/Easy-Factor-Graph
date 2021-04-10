/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_INSERT_CAPABLE_H
#define EFG_NODES_INSERT_CAPABLE_H

#include <distribution/Distribution.h>

namespace EFG::nodes {
    class InsertCapable {
    public:
        virtual void Insert(distribution::DistributionPtr factor) = 0;

    protected:
        InsertCapable() = default;
    };
}

#endif
