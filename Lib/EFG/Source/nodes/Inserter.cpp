/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/Inserter.h>
#include <Error.h>

namespace EFG::nodes {
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
        categoric::VariablePtr var = *factor->getGroup().getVariables().begin();
        auto itN = this->nodes.find(var);
        if (itN == this->nodes.end()) {
            // add this variable to the container
            auto res = this->nodes.emplace(var, var);
            itN = res.first;
            this->hidden.clusters.push_back({ &itN->second });
        }
        else {
            if (var.get() != itN->first.get()) {
                throw Error("New factor to insert should refer to the same variable stored inside the model");
            }
        }
        itN->second.unaryFactors.emplace_back(factor);
    }

    void Inserter::InsertBinary(distribution::DistributionPtr factor) {
        bool varAIsNew = false;
        categoric::VariablePtr varA = *factor->getGroup().getVariables().begin();
        auto itNA = this->nodes.find(varA);
        if (itNA == this->nodes.end()) {
            // add this variable to the container
            auto res = this->nodes.emplace(varA, varA);
            itNA = res.first;
            varAIsNew = true;
        }
        else {
            if (varA.get() != itNA->first.get()) {
                throw Error("New factor to insert should refer to the same variable stored inside the model");
            }
        }

        bool varBIsNew = false;
        categoric::VariablePtr varB = *factor->getGroup().getVariables().rbegin();
        auto itNB = this->nodes.find(varB);
        if (itNB == this->nodes.end()) {
            // add this variable to the container
            auto res = this->nodes.emplace(varB, varB);
            itNB = res.first;
            varBIsNew = true;
        }
        else {
            if (varB.get() != itNB->first.get()) {
                throw Error("New factor to insert should refer to the same variable stored inside the model");
            }
        }

        if (varAIsNew && varBIsNew) {
            this->hidden.clusters.push_back({ &itNA->second, &itNB->second });
        }
        else if (!varAIsNew && !varBIsNew) {
            if (this->binaryFactors.find(factor) != this->binaryFactors.end()) {
                throw Error("The variables involved in the passed factor are already connected by an existing factor");

            }
            auto clusterA = this->hidden.find(itNA->second);
            auto clusterB = this->hidden.find(itNB->second);
            std::set<Node*> merged = *clusterA;
            add(merged, *clusterB);
            this->hidden.clusters.erase(clusterA);
            this->hidden.clusters.erase(clusterB);
            this->hidden.clusters.push_back(merged);
        }
        else if (varAIsNew && !varBIsNew) {
            auto cluster = this->hidden.find(itNB->second);
            cluster->emplace(&itNA->second);
        }
        else if (!varAIsNew && varBIsNew) {
            auto cluster = this->hidden.find(itNA->second);
            cluster->emplace(&itNB->second);
        }

        this->binaryFactors.emplace(factor);
        itNA->second.activeConnections.emplace(&itNB->second, factor);
        itNB->second.activeConnections.emplace(&itNA->second, factor);
    }
}
