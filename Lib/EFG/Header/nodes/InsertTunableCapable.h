/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_INSERT_TUNABLE_CAPABLE_H
#define EFG_NODES_INSERT_TUNABLE_CAPABLE_H

#include <nodes/InsertCapable.h>
#include <distribution/factor/modifiable/FactorExponential.h>

namespace EFG::nodes {
    class InsertTunableCapable 
        : public InsertCapable
        , virtual public StructureTunableAware {
    public:
        // here is copied using the variables stored inside this model
        void insertTunableCopy(const distribution::factor::modif::FactorExponential& factor);
        // here is copied using the variables stored inside this model
        void insertTunableCopy(const distribution::factor::modif::FactorExponential& factor, const categoric::Group& potentialSharingWeight);

    protected:
        virtual void insertTunable(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert);
        virtual void insertTunable(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert, const categoric::Group& potentialSharingWeight);

        void absorb(const StructureTunableAware& toAbsorb, const bool& useCopyInsertion) override;

    private:
        std::map<std::shared_ptr<distribution::factor::modif::FactorExponential>, std::size_t>::const_iterator findSharingFactor(const categoric::Group& potentialSharingWeight) const;
    };
}

#endif
