/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_INSERTER_H
#define EFG_NODES_INSERTER_H

#include <nodes/NodesContainer.h>

namespace EFG::nodes {
    class Inserter : virtual public NodesContainer {
    public:
        virtual void Insert(distribution::DistributionPtr factor);

    protected:
        std::set<distribution::DistributionPtr> binaryFactors;
    };
}

#endif
