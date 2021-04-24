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
        /**
         * @brief insert a copy of the passed tunable factor.
         * A new tunable cluster is created containing only the passed factor.
         */
        void insertTunableCopy(const distribution::factor::modif::FactorExponential& factor);
        /**
         * @brief insert a copy of the passed tunable factor, sharing the weight with an already inserted one.
         * @param the factor to insert
         * @param the set of variables identifying the potential whose weight is to share
         */
        void insertTunableCopy(const distribution::factor::modif::FactorExponential& factor, const std::set<categoric::VariablePtr>& potentialSharingWeight);

    protected:
        virtual void insertTunable(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert);
        virtual void insertTunable(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert, const std::set<categoric::VariablePtr>& potentialSharingWeight);

        void absorb(const StructureTunableAware& toAbsorb, const bool& useCopyInsertion) override;

    private:
        std::map<std::shared_ptr<distribution::factor::modif::FactorExponential>, std::size_t>::const_iterator findSharingFactor(const std::set<categoric::VariablePtr>& potentialSharingWeight) const;
    };
}

#endif
