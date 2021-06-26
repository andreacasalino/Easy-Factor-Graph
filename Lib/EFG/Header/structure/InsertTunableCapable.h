/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_STRUCTURE_INSERT_TUNABLE_CAPABLE_H
#define EFG_STRUCTURE_INSERT_TUNABLE_CAPABLE_H

#include <structure/InsertCapable.h>
#include <distribution/factor/modifiable/FactorExponential.h>

namespace EFG::strct {
    class InsertTunableCapable 
        : public InsertCapable
        , virtual public StructureTunableAware {
    public:
        /**
         * @brief insert a copy of the passed tunable factor.
         * A new tunable cluster is created containing only the passed factor.
         * @throw if the factor is neither unary or binary
         * @throw in case the factor is binary and connects some already connected variables
         * @throw when there is already a variable in the model with the same name of a one into
         * the factor domain, but not represented by the same object
         */
        void insertTunableCopy(const distribution::factor::modif::FactorExponential& factor);
        /**
         * @brief insert a copy of the passed tunable factor, sharing the weight with an already inserted one.
         * @param the factor to insert
         * @param the set of variables identifying the potential whose weight is to share
         * @throw when the potential whose weight must be shared is not found
         * @throw if the factor is neither unary or binary
         * @throw in case the factor is binary and connects some already connected variables
         * @throw when there is already a variable in the model with the same name of a one into
         * the factor domain, but with a different size
         */
        void insertTunableCopy(const distribution::factor::modif::FactorExponential& factor, const std::set<categoric::VariablePtr>& potentialSharingWeight);

    protected:
        virtual void insertTunable(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert);
        virtual void insertTunable(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert, const std::set<categoric::VariablePtr>& potentialSharingWeight);

        void absorb(const StructureTunableAware& toAbsorb, const bool& useCopyInsertion) override;

        std::map<std::shared_ptr<distribution::factor::modif::FactorExponential>, std::size_t>::const_iterator findSharingFactor(const std::set<categoric::VariablePtr>& potentialSharingWeight) const;
    };
}

#endif
