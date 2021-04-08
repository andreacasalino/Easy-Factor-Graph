/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/Inserter.h>
#include <Error.h>

namespace EFG::nodes {
    void Inserter::Insert(distribution::DistributionPtr factor) {
        if (factor->getGroup().getVariables().size() == 1) {
            
            return;
        }


        if (factor->getGroup().getVariables().size() == 2) {
            // if introduces new variables add the new hidden cluster
            //todo

            // if connected to an existing variable add the new var to that cluster
            // todo

            return;
        }

        throw Error("Only binary or unary factors can be added");
    }
}
