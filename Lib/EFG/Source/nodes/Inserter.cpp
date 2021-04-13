/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/Inserter.h>
#include <distribution/factor/const/Factor.h>
#include <distribution/factor/modifiable/Factor.h>
#include <distribution/factor/modifiable/FactorExponential.h>
#include <Error.h>
#include <algorithm>

namespace EFG::nodes {
    Inserter::FindOrInsertionResult Inserter::findOrInsertNode(categoric::VariablePtr variable) {
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
        auto itEv = this->evidences.find(variable);
        if(itEv != this->evidences.end()) {
            return { &itNode->second, &itEv->second };
        }
        return {&itNode->second, nullptr};
    }

    void Inserter::Insert(distribution::DistributionPtr factor) {
        if (3 <= factor->getGroup().getVariables().size()) {
            throw Error("Only binary or unary factors can be added");
        }

        if (factor->getGroup().getVariables().size() == 1) {
            this->InsertUnary(factor);
        }
        else {
            this->InsertBinary(factor);
        }

        this->lastPropagation.reset();

        distribution::factor::cnst::Factor* factorPt = dynamic_cast<distribution::factor::cnst::Factor*>(factor.get());
        if (nullptr != factorPt) {
            this->factors.emplace(factorPt);
            return;
        }

        distribution::factor::cnst::FactorExponential* factorExpPt = dynamic_cast<distribution::factor::cnst::FactorExponential*>(factor.get());
        if (nullptr != factorExpPt) {
            this->factorsExp.emplace(factorExpPt);
            return;
        }

        throw Error("unrecognized factor type");
    }

    void Inserter::InsertUnary(distribution::DistributionPtr factor) {
        auto nodeInfo = this->findOrInsertNode(*factor->getGroup().getVariables().begin());
        nodeInfo.nodePtr->unaryFactors.emplace_back(factor);
    }

    // std::map<Node*, Connection>
    void createConnections(Node* nodeA, Node* nodeB, distribution::DistributionPtr factor, const bool& activeDisabled) {
        if (activeDisabled) {
            auto connA = nodeA->activeConnections.emplace(nodeB, factor);
            auto connB = nodeB->activeConnections.emplace(nodeA, factor);
            connA.first->second.twin = &connB.first->second;
            connB.first->second.twin = &connA.first->second;
        }
        else {
            auto connA = nodeA->disabledConnections.emplace(nodeB, factor);
            auto connB = nodeB->disabledConnections.emplace(nodeA, factor);
            connA.first->second.twin = &connB.first->second;
            connB.first->second.twin = &connA.first->second;
        }
    };

    void Inserter::connectHidden(Node* nodeA, Node* nodeB, distribution::DistributionPtr factor) {
        createConnections(nodeA, nodeB, factor, true);
        // update clusters
        auto clusterA = this->hidden.find(*nodeA);
        auto clusterB = this->hidden.find(*nodeB);
        if (clusterA == clusterB) {
            return;
        }
        std::set<Node*> merged = *clusterA;
        copyCluster(merged, *clusterB);
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
        createConnections(nodeA, nodeB, factor, false);
    };

    void Inserter::connectHiddenObserved(Node* hidden, Node* observed, distribution::DistributionPtr factor, std::size_t observation) {
        createConnections(hidden, observed, factor, false);
        hidden->disabledConnections.find(observed)->second.factor = std::make_unique<distribution::factor::cnst::Factor>(*factor, Combination({ observation }), categoric::Group(observed->variable));
    };

    void Inserter::InsertBinary(distribution::DistributionPtr factor) {
        auto nodeAInfo = this->findOrInsertNode(*factor->getGroup().getVariables().begin());
        auto nodeBInfo = this->findOrInsertNode(*factor->getGroup().getVariables().rbegin());

        if (nodeAInfo.nodePtr->activeConnections.find(nodeBInfo.nodePtr) != nodeAInfo.nodePtr->activeConnections.end()) {
            throw Error("The variables involved in the passed factor are already connected by an existing factor");
        }
        if (nodeAInfo.nodePtr->disabledConnections.find(nodeBInfo.nodePtr) != nodeAInfo.nodePtr->disabledConnections.end()) {
            throw Error("The variables involved in the passed factor are already connected by an existing factor");
        }

        if ((nodeAInfo.evidence == nullptr) && (nodeBInfo.evidence == nullptr)) {
            this->connectHidden(nodeAInfo.nodePtr, nodeBInfo.nodePtr, factor);
            return;
        }
        
        if ((nodeAInfo.evidence != nullptr) && (nodeBInfo.evidence != nullptr)) {
            this->connectObserved(nodeAInfo.nodePtr, nodeBInfo.nodePtr, factor);
            return;
        }

        if (nodeAInfo.evidence == nullptr) {
            // A is hidden, B is observed
            this->connectHiddenObserved(nodeAInfo.nodePtr, nodeBInfo.nodePtr, factor, *nodeBInfo.evidence);
        }
        // A is observed, B is hidden
        this->connectHiddenObserved(nodeBInfo.nodePtr, nodeAInfo.nodePtr, factor, *nodeAInfo.evidence);
    }

    categoric::Group Inserter::convert(const categoric::Group& toConvert) {
        auto findAndCheck = [this](const categoric::VariablePtr& var) {
            auto itVar = this->nodes.find(var);
            if (itVar == this->nodes.end()) {
                return this->findOrInsertNode(var).nodePtr->variable;
            }
            if (itVar->first->size() != var->size()) {
                throw Error("variable with size mismatch");
            }
            return itVar->first;
        };

        auto itV = toConvert.getVariables().begin();
        categoric::Group converted(findAndCheck(*itV));
        ++itV;
        std::for_each(itV, toConvert.getVariables().end(), [&converted,&findAndCheck](const categoric::VariablePtr& var) {
            converted.add(findAndCheck(var));
        });
        return converted;
    }

    void Inserter::Insert(const distribution::Distribution& factor) {
        std::shared_ptr<distribution::factor::modif::Factor> distr = std::make_shared<distribution::factor::modif::Factor>(factor);
        distr->replaceGroup(this->convert(factor.getGroup()));
        this->Insert(distr);
    }

    void Inserter::Insert(const distribution::factor::cnst::FactorExponential& factor) {
        std::shared_ptr<distribution::factor::modif::FactorExponential> distr = std::make_shared<distribution::factor::modif::FactorExponential>(factor);
        distr->replaceGroup(this->convert(factor.getGroup()));
        this->Insert(distr);
    }
}
