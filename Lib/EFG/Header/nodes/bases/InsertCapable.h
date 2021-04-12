/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_INSERT_CAPABLE_H
#define EFG_NODES_INSERT_CAPABLE_H

#include <nodes/bases/Base.h>
#include <distribution/factor/const/FactorExponential.h>

namespace EFG::nodes {
    class InsertCapable : virtual public Base {
    public:
        virtual void Insert(distribution::DistributionPtr toInsert) = 0;

        // here is copied using the variables stored inside this model
        virtual void Insert(const distribution::Distribution& factor) = 0;

        // here is copied using the variables stored inside this model
        virtual void Insert(const distribution::factor::cnst::FactorExponential& factor) = 0;
    };
}

#endif
