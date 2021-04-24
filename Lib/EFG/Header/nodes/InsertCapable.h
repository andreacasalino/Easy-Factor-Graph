/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_INSERT_CAPABLE_H
#define EFG_NODES_INSERT_CAPABLE_H

#include <nodes/bases/NodesAware.h>
#include <nodes/bases/EvidenceAware.h>
#include <nodes/bases/BeliefAware.h>
#include <nodes/bases/StructureAware.h>
#include <nodes/bases/StructureTunableAware.h>
#include <Error.h>

namespace EFG::nodes {
    class InsertCapable
        : virtual public NodesAware
        , virtual public EvidenceAware
        , virtual public BeliefAware
        , virtual public StructureAware {
    public:
        /**
         * @brief insert the passed factor. 
         */
        void insert(std::shared_ptr<distribution::factor::cnst::Factor> factor);
        /**
         * @brief insert a copy of the passed factor.
         */
        void insertCopy(const distribution::Distribution& factor);

        /**
         * @brief insert the passed epxonential factor.
         */
        void insert(std::shared_ptr<distribution::factor::cnst::FactorExponential> factor);
        /**
         * @brief insert a copy of the passed exponential factor.
         */
        void insertCopy(const distribution::factor::cnst::FactorExponential& factor);

        /**
         * @brief insert all the factors contained in the passed model
         * @param the model to absorb
         * @param when passing true the factors from the passed model are copied, otherwise the shared pointers are copied
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
