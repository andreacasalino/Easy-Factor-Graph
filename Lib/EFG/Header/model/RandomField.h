/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_MODEL_RANDOM_FIELD_H
#define EFG_MODEL_RANDOM_FIELD_H

#include <structure/BeliefPropagator.h>
#include <structure/EvidenceChanger.h>
#include <structure/EvidenceSetter.h>
#include <structure/GibbsSampler.h>
#include <structure/InsertTunableCapable.h>
#include <structure/QueryHandler.h>
#include <train/Trainable.h>

namespace EFG::model {
    class RandomField
        : public strct::BeliefPropagator
        , public strct::EvidencesChanger
        , public strct::EvidencesSetter
        , public strct::GibbsSampler
        , public strct::InsertTunableCapable
        , public strct::QueryHandler
        , public train::Trainable {
    public:
        RandomField() = default;

        template<typename Model>
        explicit RandomField(const Model& o) {
            this->absorbModel(o);
        };

        RandomField(const RandomField& o) {
            this->absorbModel(o, true);
        };

        /**
         * @brief insert the passed tunable factor.
         */
        void insertTunable(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert) override;
        /**
         * @brief insert the passed tunable factor, sharing the weight with an already inserted one.
         * @param the factor to insert
         * @param the set of variables identifying the potential whose weight is to share
         */
        void insertTunable(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert, const std::set<categoric::VariablePtr>& potentialSharingWeight) override;

        std::vector<float> getGradient(train::TrainSetPtr trainSet) override;
    };
}

#endif
