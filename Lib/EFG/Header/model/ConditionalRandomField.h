/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_MODEL_CONDITIONAL_RANDOM_FIELD_H
#define EFG_MODEL_CONDITIONAL_RANDOM_FIELD_H

#include <nodes/BeliefPropagator.h>
#include <nodes/EvidenceChanger.h>
#include <nodes/EvidenceSetter.h>
#include <nodes/GibbsSampler.h>
#include <nodes/InsertTunableCapable.h>
#include <nodes/QueryHandler.h>
#include <train/Trainable.h>

namespace EFG::model {
    class ConditionalRandomField
        : public nodes::BeliefPropagator
        , private nodes::EvidencesChanger
        , public nodes::EvidencesSetter
        , public nodes::GibbsSampler
        , public nodes::InsertTunableCapable
        , public nodes::QueryHandler
        , public train::Trainable {
    public:
        ConditionalRandomField() = delete;
        ConditionalRandomField(const std::string& xmlPath);
        // template<typename Model>
        // ConditionalRandomField(const Model& model);

        void insertTunable(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert) override;
        void insertTunable(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert, const categoric::Group& potentialSharingWeight) override;

    private:
        train::TrainHandlerPtr makeHandler(std::shared_ptr<distribution::factor::modif::FactorExponential> factor) override;

        std::vector<float> getGradient(train::TrainSetPtr trainSet) override;
    };
}

#endif
