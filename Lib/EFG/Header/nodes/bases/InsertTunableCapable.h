/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_INSERT_TUNABLE_CAPABLE_H
#define EFG_NODES_INSERT_TUNABLE_CAPABLE_H

#include <nodes/bases/InsertCapable.h>
#include <distribution/factor/modifiable/FactorExponential.h>

namespace EFG::nodes {
    class InsertTunableCapable : virtual public InsertCapable {
    public:
        virtual void Insert(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert) = 0;

        virtual void Insert(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert, const categoric::Group& potentialSharingWeight) = 0;
    };
}

#endif
