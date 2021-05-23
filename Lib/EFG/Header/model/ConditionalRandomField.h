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
#include <io/FilePath.h>

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

        /**
         * @throw in case no evidences are present in the passed model
         */
        template<typename Model>
        explicit ConditionalRandomField(const Model& o) {
            this->absorbOther(o);
        };

        ConditionalRandomField(const ConditionalRandomField& o) {
            this->absorbOther(o);
        };

        /**
         * @brief import the model from an xml file
         * @param the path of the xml to read
         * @throw in case no evidences are set in the file
         */
        ConditionalRandomField(const io::FilePath& filePath);

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

    private:
        train::TrainHandlerPtr makeHandler(std::shared_ptr<distribution::factor::modif::FactorExponential> factor) override;

        std::vector<float> getGradient(train::TrainSetPtr trainSet) override;

        template<typename Model>
        void absorbOther(const Model& o) {
            const nodes::EvidenceAware* evPtr = dynamic_cast<const nodes::EvidenceAware*>(&o);
            if (nullptr == evPtr) {
                throw Error("the passed model is not evidence aware");
            }
            const auto& ev = evPtr->getEvidences();
            if (ev.empty()) {
                throw Error("the passed model should have at least 1 evidence");
            }
            this->absorbModel(o, true);
            std::map<std::string, std::size_t> ev2;
            for (auto it = ev.begin(); it != ev.end(); ++it) {
                ev2.emplace(it->first->name(), it->second);
            }
            this->resetEvidences(ev2);
            this->regenerateHandlers();
        };

        // regenerate after knowing the eveidence set
        void regenerateHandlers();
    };
}

#endif
