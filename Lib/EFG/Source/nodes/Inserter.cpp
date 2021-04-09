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

    Inserter::NodeInfo Inserter::checkAddNode(categoric::VariablePtr variable) {
        bool varIsNew = false;
        auto itNode = this->nodes.find(variable);
        if (itNode == this->nodes.end()) {
            // add this variable to the container
            auto res = this->nodes.emplace(variable, variable);
            itNode = res.first;
            varIsNew = true;
        }
        else {
            if (variable.get() != itNode->first.get()) {
                throw Error("New factor to insert should refer to the same variable stored inside the model");
            }
        }
        return { varIsNew, &itNode->second };
    }

    void Inserter::Insert(distribution::DistributionPtr factor) {
        if (factor->getGroup().getVariables().size() == 1) {
            this->InsertUnary(factor);
            return;
        }

        if (factor->getGroup().getVariables().size() == 2) {
            this->InsertBinary(factor);
            return;
        }

        throw Error("Only binary or unary factors can be added");
    }

    void Inserter::InsertUnary(distribution::DistributionPtr factor) {
        auto nodeInfo = this->checkAddNode(*factor->getGroup().getVariables().begin());
        nodeInfo.nodePtr->unaryFactors.emplace_back(factor);
        if (nodeInfo.wasNew) {
            this->hidden.clusters.push_back({nodeInfo.nodePtr });
        }
    }

    void Inserter::InsertBinary(distribution::DistributionPtr factor) {
        auto nodeAInfo = this->checkAddNode(*factor->getGroup().getVariables().begin());
        auto nodeBInfo = this->checkAddNode(*factor->getGroup().getVariables().rbegin());

        VariablePair factorKey(nodeAInfo.nodePtr->variable->name(), nodeBInfo.nodePtr->variable->name());

        this->binaryFactors.emplace(factorKey, factor);

        auto connect1 = [&nodeAInfo, &nodeBInfo, &factor]() {
            nodeAInfo.nodePtr->activeConnections.emplace(nodeBInfo.nodePtr, factor);
            nodeBInfo.nodePtr->activeConnections.emplace(nodeAInfo.nodePtr, factor);
        };

        auto connect2 = [&factor](Node* observed, Node* hidden, std::size_t obs) {
            observed->disabledConnections.emplace(hidden, factor);

            Connection newConnection(factor, std::make_unique<distribution::factor::cnst::Factor>(*factor, Combination({ obs }), categoric::Group(hidden->variable)));
            hidden->disabledConnections.emplace(observed, std::move(newConnection));
        };


        if (nodeAInfo.wasNew && nodeBInfo.wasNew) {
            this->hidden.clusters.push_back({ nodeAInfo.nodePtr , nodeBInfo.nodePtr });
            connect1();
            return;
        }

        if (!nodeAInfo.wasNew && !nodeBInfo.wasNew) {
            if (this->binaryFactors.find(factorKey) != this->binaryFactors.end()) {
                throw Error("The variables involved in the passed factor are already connected by an existing factor");
            }
            auto clusterA = this->hidden.find(*nodeAInfo.nodePtr);
            auto clusterB = this->hidden.find(*nodeBInfo.nodePtr);

            if ((clusterA == this->hidden.clusters.end()) && (clusterA == this->hidden.clusters.end())) {
                // both are observed
                nodeAInfo.nodePtr->disabledConnections.emplace(nodeBInfo.nodePtr, factor);
                nodeBInfo.nodePtr->disabledConnections.emplace(nodeAInfo.nodePtr, factor);
                return;
            }
            else if ((clusterA != this->hidden.clusters.end()) && (clusterA != this->hidden.clusters.end())) {
                // both are hidden
                std::set<Node*> merged = *clusterA;
                add(merged, *clusterB);
                this->hidden.clusters.erase(clusterA);
                this->hidden.clusters.erase(clusterB);
                this->hidden.clusters.push_back(merged);
                connect1();
                return;
            }
            
            Node* observed, * hidden;
            std::size_t obs;
            if(clusterA == this->hidden.clusters.end()) {
                auto itAOb = this->evidences.find(nodeAInfo.nodePtr->variable);
                observed = nodeAInfo.nodePtr;
                hidden = nodeBInfo.nodePtr;
                obs = itAOb->second;
            }
            else {
                auto itBOb = this->evidences.find(nodeBInfo.nodePtr->variable);
                observed = nodeBInfo.nodePtr;
                hidden = nodeAInfo.nodePtr;
                obs = itBOb->second;
            }
            connect2(observed, hidden, obs);
            return;
        }

        if (nodeAInfo.wasNew && !nodeBInfo.wasNew) {
            auto clusterB = this->hidden.find(*nodeBInfo.nodePtr);
            if (clusterB == this->hidden.clusters.end()) {
                // B is observed
                this->hidden.clusters.push_back({ nodeAInfo.nodePtr });
                auto itBOb = this->evidences.find(nodeBInfo.nodePtr->variable);
                connect2(nodeBInfo.nodePtr, nodeAInfo.nodePtr, itBOb->second);
            }
            else {
                // B is hidden
                clusterB->emplace(nodeAInfo.nodePtr);
                connect1();
            }
            return;
        }
 
        auto clusterA = this->hidden.find(*nodeAInfo.nodePtr);
        if (clusterA == this->hidden.clusters.end()) {
            // A is observed
            this->hidden.clusters.push_back({ nodeBInfo.nodePtr });
            auto itAOb = this->evidences.find(nodeAInfo.nodePtr->variable);
            connect2(nodeAInfo.nodePtr, nodeBInfo.nodePtr, itAOb->second);
        }
        else {
            // A is hidden
            clusterA->emplace(nodeBInfo.nodePtr);
            connect1();
        }
    }
}
