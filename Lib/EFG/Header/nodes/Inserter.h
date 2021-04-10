/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_INSERTER_H
#define EFG_NODES_INSERTER_H

#include <nodes/NodesContainer.h>
#include <nodes/InsertCapable.h>

namespace EFG::nodes {
    class Inserter 
        : virtual public NodesContainer
        , virtual public InsertCapable {
    public:
        void Insert(distribution::DistributionPtr factor) override;

    protected:
        class VariablePair {
        public:
            VariablePair(const std::string& nameA, const std::string& nameB);

            inline bool operator<(const VariablePair& o) const { return this->pairName < o.pairName; };

        private:
            std::string pairName;
        };
        std::map<VariablePair, distribution::DistributionPtr> binaryFactors;

    private:
        void InsertUnary(distribution::DistributionPtr factor);
        void InsertBinary(distribution::DistributionPtr factor);

        struct NodeInfo {
            Node* nodePtr;
            const std::size_t* evidence;
        };
        NodeInfo findOrInsertNode(categoric::VariablePtr variable);
        void connectHidden(Node* nodeA, Node* nodeB, distribution::DistributionPtr factor);
        void connectObserved(Node* nodeA, Node* nodeB, distribution::DistributionPtr factor);
        void connectHiddenObserved(Node* hidden, Node* observed, std::size_t observation, distribution::DistributionPtr factor);
    };
}

#endif
