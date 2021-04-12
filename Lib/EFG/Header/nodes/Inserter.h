/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_INSERTER_H
#define EFG_NODES_INSERTER_H

#include <nodes/bases/NodesAware.h>
#include <nodes/bases/EvidenceAware.h>
#include <nodes/bases/BeliefAware.h>
#include <nodes/bases/InsertCapable.h>
#include <nodes/bases/StructureAware.h>

namespace EFG::nodes {
    class Inserter
        : virtual public NodesAware
        , virtual public EvidenceAware
        , virtual public BeliefAware
        , virtual public InsertCapable
        , virtual public StructureAware {
    public:
        void Insert(distribution::DistributionPtr factor) override;

        void Insert(const distribution::Distribution& factor) override;

        void Insert(const distribution::factor::cnst::FactorExponential& factor) override;

    private:
        void InsertUnary(distribution::DistributionPtr factor);
        void InsertBinary(distribution::DistributionPtr factor);

        categoric::Group convert(const categoric::Group& toConvert);

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
