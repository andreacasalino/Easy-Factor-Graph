/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_INSERTION_MANAGER_H
#define EFG_NODES_INSERTION_MANAGER_H

#include <nodes/NodesBase.h>

namespace EFG::nodes {
    class InsertionManager : virtual public NodesBase {
    public:
        virtual void Insert(distribution::DistributionPtr toInsert);

    protected:
        inline const std::set<distribution::DistributionPtr>& getBinaryFactorS() final { return this->binaryFactors; }; 
        inline const std::set<distribution::DistributionPtr>& getUnaryFactorS() final { return this->unaryFactors; };

    private:
        std::set<distribution::DistributionPtr> binaryFactors;
        std::set<distribution::DistributionPtr> unaryFactors;
    };
}

#endif
