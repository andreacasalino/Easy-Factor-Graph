/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/QueryHandler.h>
#include <distribution/factor/modifiable/Factor.h>
#include <distribution/DistributionIterator.h>
#include <algorithm>
#include <Error.h>

namespace EFG::nodes {
    distribution::factor::cnst::Factor mergeMessages(const Node& node) {
        std::set<const distribution::Distribution*> toMerge;
        std::for_each(node.unaryFactors.begin(), node.unaryFactors.end(), [&toMerge](const distribution::DistributionPtr& d) {
            toMerge.emplace(d.get());
        });
        for (auto it = node.activeConnections.begin(); it != node.activeConnections.end(); ++it) {
            toMerge.emplace(it->second.message2This.get());
        }
        distribution::factor::modif::Factor factor(toMerge);
        factor.emplaceEntireDomain();
        return factor;
    }

    std::vector<float> QueryHandler::getMarginalDistribution(const std::string& var) {
        auto itN = this->nodes.find(categoric::makeVariable(2, var));
        if (itN == this->nodes.end()) {
            throw Error("non existent variable");
        }
        if (BeliefPropagationInfo::Sum != this->lastPropagationDone) {
            this->propagateBelief(PropagationKind::Sum);
        }
        return mergeMessages(itN->second).getProbabilities();
    }

    // distribution::factor::cnst::Factor getJointMarginalDistribution(const std::vector<std::string>& subgroup)

    std::size_t getMAPnode(const Node& node) {
        auto iter = mergeMessages(node).getIterator();
        std::size_t maxPos = 0, pos = 0;
        float maxVal = iter.getImage(), val;
        ++iter;
        ++pos;
        iterator::forEach(iter, [&](distribution::DistributionIterator& i) {
            float val = i.getImage();
            if (val > maxVal) {
                maxPos = pos;
                maxVal = val;
            }
            ++pos;
        });
        return maxPos;
    }

    std::size_t QueryHandler::getMAP(const std::string& var) {
        auto itN = this->nodes.find(categoric::makeVariable(2, var));
        if (itN == this->nodes.end()) {
            throw Error("non existent variable");
        }
        if (BeliefPropagationInfo::Sum != this->lastPropagationDone) {
            this->propagateBelief(PropagationKind::MAP);
        }
        return getMAPnode(itN->second);
    }

    std::vector<size_t> QueryHandler::getHiddenSetMAP() {
        if (BeliefPropagationInfo::Sum != this->lastPropagationDone) {
            this->propagateBelief(PropagationKind::MAP);
        }
        auto hiddenVars = this->getHiddenVariables();
        std::vector<size_t> MAP;
        MAP.reserve(hiddenVars.size());
        std::for_each(hiddenVars.begin(), hiddenVars.end(), [this, &MAP](const categoric::VariablePtr& v) {
            MAP.push_back(getMAPnode(this->nodes.find(v)->second));
        });
        return MAP;
    }
}
