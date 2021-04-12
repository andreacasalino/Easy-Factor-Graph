/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_INSERT_TUNABLE_CAPABLE_H
#define EFG_NODES_INSERT_TUNABLE_CAPABLE_H

#include <nodes/bases/Base.h>
#include <distribution/factor/modifiable/FactorExponential.h>

namespace EFG::nodes {
    class InsertTunableCapable : virtual public Base {
    public:
        virtual void Insert(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert) = 0;

        // here is copied using the variables stored inside this model
        virtual void Insert(const distribution::factor::modif::FactorExponential& factor) = 0;

        virtual void Insert(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert, const categoric::Group& potentialSharingWeight) = 0;

        // here is copied using the variables stored inside this model
        virtual void Insert(const distribution::factor::modif::FactorExponential& factor, const categoric::Group& potentialSharingWeight) = 0;
    };
}

#endif
