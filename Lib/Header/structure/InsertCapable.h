/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_STRUCTURE_INSERT_CAPABLE_H
#define EFG_STRUCTURE_INSERT_CAPABLE_H

#include <structure/components/NodesAware.h>
#include <structure/components/EvidenceAware.h>
#include <structure/components/BeliefAware.h>
#include <structure/components/StructureAware.h>
#include <structure/components/StructureTunableAware.h>
#include <Error.h>

namespace EFG::strct {
    class InsertCapable
        : virtual public NodesAware
        , virtual public EvidenceAware
        , virtual public BeliefAware
        , virtual public StructureAware {
    public:
        /**
         * @brief insert the passed factor. 
         * @throw if the factor is neither unary or binary
         * @throw in case the factor is binary and connects some already connected variables
         * @throw when there is already a variable in the model with the same name of a one into
         * the factor domain, but not represented by the same object
         */
        void insert(std::shared_ptr<distribution::factor::cnst::Factor> factor);
        /**
         * @brief insert a copy of the passed factor.
         * @throw if the factor is neither unary or binary
         * @throw in case the factor is binary and connects some already connected variables
         * @throw when there is already a variable in the model with the same name of a one into
         * the factor domain, but with a different size
         */
        void insertCopy(const distribution::Distribution& factor);

        /**
         * @brief insert the passed epxonential factor.
         * @throw if the factor is neither unary or binary
         * @throw in case the factor is binary and connects some already connected variables
         * @throw when there is already a variable in the model with the same name of a one into
         * the factor domain, but not represented by the same object
         */
        void insert(std::shared_ptr<distribution::factor::cnst::FactorExponential> factor);
        /**
         * @brief insert a copy of the passed exponential factor.
         * @throw if the factor is neither unary or binary
         * @throw in case the factor is binary and connects some already connected variables
         * @throw when there is already a variable in the model with the same name of a one into
         * the factor domain, but with a different size
         */
        void insertCopy(const distribution::factor::cnst::FactorExponential& factor);

        /**
         * @brief insert all the factors contained in the passed model
         * @param the model to absorb
         * @param when passing true the factors from the passed model are copied, otherwise the shared pointers are copied
         * @throw if the passed model does not expose a StructureAware interface
         */
        template<typename Model>
        void absorbModel(const Model& model, const bool& useCopyInsertion = false) {
            const StructureAware* strct = dynamic_cast<const StructureAware*>(&model);
            if (nullptr == strct) {
                throw Error("the model to absorb should be at least StructureAware");
            }
            this->absorb(*strct, useCopyInsertion);
            const StructureTunableAware* strctTunab = dynamic_cast<const StructureTunableAware*>(&model);
            if (nullptr != strctTunab) {
                this->absorb(*strctTunab, useCopyInsertion);
            }
        };

    protected:
        std::set<categoric::VariablePtr> convertUsingLocals(const std::set<categoric::VariablePtr>& toConvert);

        void absorb(const StructureAware& toAbsorb, const bool& useCopyInsertion);
        virtual void absorb(const StructureTunableAware& toAbsorb, const bool& useCopyInsertion);

    private:
        void insertPtr(distribution::DistributionPtr toInsert);
        void insertUnary(distribution::DistributionPtr factor);
        void insertBinary(distribution::DistributionPtr factor);

        struct FindOrInsertionResult {
            Node* nodePtr;
            const std::size_t* evidence;
        };
        FindOrInsertionResult findOrInsertNode(categoric::VariablePtr variable);

        void connectHidden(Node* nodeA, Node* nodeB, distribution::DistributionPtr factor);
        void connectObserved(Node* nodeA, Node* nodeB, distribution::DistributionPtr factor);
        void connectHiddenObserved(Node* hidden, Node* observed, distribution::DistributionPtr factor, std::size_t observation);
    };
}

#endif
