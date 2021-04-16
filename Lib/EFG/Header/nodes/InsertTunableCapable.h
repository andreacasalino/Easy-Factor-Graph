/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_INSERT_TUNABLE_CAPABLE_H
#define EFG_NODES_INSERT_TUNABLE_CAPABLE_H

#include <nodes/InsertCapable.h>
#include <nodes/bases/StructureTunableAware.h>
#include <distribution/factor/modifiable/FactorExponential.h>

namespace EFG::nodes {
    class InsertTunableCapable 
        : public InsertCapable
        , virtual public StructureTunableAware {
    public:
        // here is copied using the variables stored inside this model
        void Insert(const distribution::factor::modif::FactorExponential& factor);
        // here is copied using the variables stored inside this model
        void Insert(const distribution::factor::modif::FactorExponential& factor, const categoric::Group& potentialSharingWeight);

    protected:
        virtual void Insert(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert);
        virtual void Insert(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert, const categoric::Group& potentialSharingWeight);

    private:
        std::map<distribution::factor::modif::FactorExponential*, std::size_t>::const_iterator findSharingFactor(const categoric::Group& potentialSharingWeight) const;
    };
}

#endif
