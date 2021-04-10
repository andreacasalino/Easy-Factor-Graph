/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/Inserter.h>
#include <distribution/factor/const/Factor.h>
#include <Error.h>

namespace EFG::nodes {
    Inserter::VariablePair::VariablePair(const std::string& nameA, const std::string& nameB) {
        this->pairName.reserve(nameA.size() + nameB.size());
        if (nameA < nameB) {
            this->pairName += nameA;
            this->pairName += nameB;
        }
        else {
            this->pairName += nameB;
            this->pairName += nameA;
        }
    }

    Inserter::NodeInfo Inserter::findOrInsertNode(categoric::VariablePtr variable) {
        auto itNode = this->nodes.find(variable);
        if (itNode == this->nodes.end()) {
            // add this variable to the container
            auto res = this->nodes.emplace(variable, variable);
            // add an isolated hidden cluster with only this node
            this->hidden.clusters.push_back({&res.first->second});
            return {&res.first->second, nullptr};
        }
        // already existent node
        if (variable.get() != itNode->first.get()) {
            throw Error("New factor to insert should refer to the same variable stored inside the model");
        }
        const std::size_t* ob = nullptr;
        auto itEv = this->evidences.find(variable);
        if(itEv != this->evidences.end()) {
            ob = &itEv->second;
        }
        return {&itNode->second, ob};
    }

    void Inserter::Insert(distribution::DistributionPtr factor) {
        if (factor->getGroup().getVariables().size() == 1) {
            this->InsertUnary(factor);
            this->lastPropagationDone = BeliefPropagationInfo::NotDone;
            return;
        }

        if (factor->getGroup().getVariables().size() == 2) {
            this->InsertBinary(factor);
            this->lastPropagationDone = BeliefPropagationInfo::NotDone;
            return;
        }

        throw Error("Only binary or unary factors can be added");
    }

    void Inserter::InsertUnary(distribution::DistributionPtr factor) {
        auto nodeInfo = this->findOrInsertNode(*factor->getGroup().getVariables().begin());
        nodeInfo.nodePtr->unaryFactors.emplace_back(factor);
    }

    void Inserter::connectHidden(Node* nodeA, Node* nodeB, distribution::DistributionPtr factor) {
        nodeA->activeConnections.emplace(nodeB, factor);
        nodeB->activeConnections.emplace(nodeA, factor);
        // update clusters
        auto clusterA = this->hidden.find(*nodeA);
        auto clusterB = this->hidden.find(*nodeB);
        std::set<Node*> merged = *clusterA;
        add(merged, *clusterB);
        if (clusterA == clusterB) {
            this->hidden.clusters.erase(clusterA);
        }
        else {
            this->hidden.clusters.erase(clusterA);
            this->hidden.clusters.erase(clusterB);
        }
        this->hidden.clusters.push_back(merged);
    };

    void Inserter::connectObserved(Node* nodeA, Node* nodeB, distribution::DistributionPtr factor) {
        nodeA->disabledConnections.emplace(nodeB, factor);
        nodeB->disabledConnections.emplace(nodeA, factor);
    };

    void Inserter::connectHiddenObserved(Node* hidden, Node* observed, std::size_t observation, distribution::DistributionPtr factor) {
        observed->disabledConnections.emplace(hidden, factor);

        Connection newConnection(factor, std::make_unique<distribution::factor::cnst::Factor>(*factor, Combination({ observation }), categoric::Group(observed->variable)));
        hidden->disabledConnections.emplace(observed, std::move(newConnection));
    };

    void Inserter::InsertBinary(distribution::DistributionPtr factor) {
        VariablePair factorKey((*factor->getGroup().getVariables().begin())->name(), (*factor->getGroup().getVariables().rbegin())->name());
        if (this->binaryFactors.find(factorKey) != this->binaryFactors.end()) {
            throw Error("The variables involved in the passed factor are already connected by an existing factor");
        }

        auto nodeAInfo = this->findOrInsertNode(*factor->getGroup().getVariables().begin());
        auto nodeBInfo = this->findOrInsertNode(*factor->getGroup().getVariables().rbegin());

        this->binaryFactors.emplace(factorKey, factor);

        if ((nodeAInfo.evidence == nullptr) && (nodeBInfo.evidence == nullptr)) {
            this->connectHidden(nodeAInfo.nodePtr, nodeBInfo.nodePtr, factor);
            return;
        }
        
        if ((nodeAInfo.evidence != nullptr) && (nodeBInfo.evidence != nullptr)) {
            this->connectObserved(nodeAInfo.nodePtr, nodeBInfo.nodePtr, factor);
            return;
        }

        if (nodeAInfo.evidence != nullptr) {
            // A is observed, B is hidden
            this->connectHiddenObserved(nodeBInfo.nodePtr, nodeAInfo.nodePtr, *nodeAInfo.evidence, factor);
        }
        else {
            // A is hidden, B is observed
            this->connectHiddenObserved(nodeAInfo.nodePtr, nodeBInfo.nodePtr, *nodeBInfo.evidence, factor);
        }
    }
}
