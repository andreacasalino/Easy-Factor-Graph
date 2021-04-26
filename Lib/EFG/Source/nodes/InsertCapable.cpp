/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/InsertCapable.h>
#include <distribution/factor/const/Factor.h>
#include <distribution/factor/modifiable/Factor.h>
#include <distribution/factor/modifiable/FactorExponential.h>
#include <Error.h>
#include <algorithm>

namespace EFG::nodes {
    InsertCapable::FindOrInsertionResult InsertCapable::findOrInsertNode(categoric::VariablePtr variable) {
        auto itNode = this->nodes.find(variable);
        if (itNode == this->nodes.end()) {
            // add this variable to the container
            auto res = this->nodes.emplace(variable, variable);
            // add an isolated hidden cluster with only this node
            this->hidden.clusters.push_back({ &res.first->second });
            return { &res.first->second, nullptr };
        }
        // already existent node
        if (variable.get() != itNode->first.get()) {
            throw Error("New factor to insert should refer to the same variable stored inside the model");
        }
        auto itEv = this->evidences.find(variable);
        if (itEv != this->evidences.end()) {
            return { &itNode->second, &itEv->second };
        }
        return { &itNode->second, nullptr };
    }

    void InsertCapable::insertPtr(distribution::DistributionPtr factor) {
        if (3 <= factor->getGroup().getVariables().size()) {
            throw Error("Only binary or unary factors can be added");
        }

        if (factor->getGroup().getVariables().size() == 1) {
            this->insertUnary(factor);
        }
        else {
            this->insertBinary(factor);
        }

        this->lastPropagation.reset();
    }

    void InsertCapable::insertUnary(distribution::DistributionPtr factor) {
        auto nodeInfo = this->findOrInsertNode(*factor->getGroup().getVariables().begin());
        nodeInfo.nodePtr->unaryFactors.emplace_back(factor);
    }

    // std::map<Node*, Connection>
    void createConnections(Node* nodeA, Node* nodeB, distribution::DistributionPtr factor, const bool& activeDisabled) {
        if (activeDisabled) {
            auto connA = nodeA->activeConnections.emplace(nodeB, factor);
            auto connB = nodeB->activeConnections.emplace(nodeA, factor);
        }
        else {
            auto connA = nodeA->disabledConnections.emplace(nodeB, factor);
            auto connB = nodeB->disabledConnections.emplace(nodeA, factor);
        }
    };

    void InsertCapable::connectHidden(Node* nodeA, Node* nodeB, distribution::DistributionPtr factor) {
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

    void InsertCapable::connectObserved(Node* nodeA, Node* nodeB, distribution::DistributionPtr factor) {
        createConnections(nodeA, nodeB, factor, false);
    };

    void InsertCapable::connectHiddenObserved(Node* hidden, Node* observed, distribution::DistributionPtr factor, std::size_t observation) {
        createConnections(hidden, observed, factor, false);
        hidden->disabledConnections.find(observed)->second.factor = std::make_unique<distribution::factor::cnst::Factor>(*factor, categoric::Combination(&observation, 1), std::set<categoric::VariablePtr>{observed->variable});
    };

    void InsertCapable::insertBinary(distribution::DistributionPtr factor) {
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

    std::set<categoric::VariablePtr> InsertCapable::convertUsingLocals(const std::set<categoric::VariablePtr>& toConvert) {
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

        auto itV = toConvert.begin();
        std::set<categoric::VariablePtr> converted = { findAndCheck(*itV) };
        ++itV;
        std::for_each(itV, toConvert.end(), [&converted, &findAndCheck](const categoric::VariablePtr& var) {
            converted.emplace(findAndCheck(var));
        });
        return converted;
    }

    void InsertCapable::insert(std::shared_ptr<distribution::factor::cnst::Factor> factor) {
        this->insertPtr(factor);
        this->factors.emplace(factor);
    }

    void InsertCapable::insertCopy(const distribution::Distribution& factor) {
        std::shared_ptr<distribution::factor::modif::Factor> distr = std::make_shared<distribution::factor::modif::Factor>(factor);
        distr->replaceGroup(categoric::Group(this->convertUsingLocals(factor.getGroup().getVariables())));
        this->insert(distr);
    }

    void InsertCapable::insert(std::shared_ptr<distribution::factor::cnst::FactorExponential> factor) {
        this->insertPtr(factor);
        this->factorsExp.emplace(factor);
    }

    void InsertCapable::insertCopy(const distribution::factor::cnst::FactorExponential& factor) {
        std::shared_ptr<distribution::factor::modif::FactorExponential> distr = std::make_shared<distribution::factor::modif::FactorExponential>(factor);
        distr->replaceGroup(categoric::Group(this->convertUsingLocals(factor.getGroup().getVariables())));
        this->insert(distr);
    }

    template<typename T>
    void replaceWithCopies(std::set<std::shared_ptr<T>>& set) {
        std::set<std::shared_ptr<T>> temp;
        std::for_each(set.begin(), set.end(), [&temp](const std::shared_ptr<T>& p){
            temp.emplace(std::make_shared<T>(*p.get()));
        });
        set = temp;
    };
    void InsertCapable::absorb(const StructureAware& toAbsorb, const bool& useCopyInsertion) {        
        auto factors = toAbsorb.getFactors();
        auto factorsExp = toAbsorb.getFactorsExp();
        if(useCopyInsertion) {
            replaceWithCopies(factors);
            replaceWithCopies(factorsExp);
        }
        for(auto it = factors.begin(); it!=factors.end(); ++it) {
            this->insert(*it);
        }
        for(auto it = factorsExp.begin(); it!=factorsExp.end(); ++it) {
            this->insert(*it);
        }
    }

    void InsertCapable::absorb(const StructureTunableAware& toAbsorb, const bool& useCopyInsertion) {
        auto clusters = toAbsorb.getFactorsTunable();
        std::set<std::shared_ptr<distribution::factor::cnst::FactorExponential>> factors;
        std::for_each(clusters.begin(), clusters.end(), [&factors](const std::vector<std::shared_ptr<distribution::factor::modif::FactorExponential>>& cl) {
            std::for_each(cl.begin(), cl.end(), [&factors](const std::shared_ptr<distribution::factor::modif::FactorExponential>& f) {
                factors.emplace(f);
            });
        });
        if (useCopyInsertion) {
            replaceWithCopies(factors);
        }
        for (auto it = factors.begin(); it != factors.end(); ++it) {
            this->insert(*it);
        }
    }
}
