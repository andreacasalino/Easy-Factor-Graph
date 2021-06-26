/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <structure/QueryHandler.h>
#include <distribution/factor/modifiable/Factor.h>
#include <distribution/DistributionIterator.h>
#include <distribution/factor/const/Indicator.h>
#include "Commons.h"
#include <algorithm>
#include <Error.h>
#include <distribution/DistributionFinder.h>
#include <structure/ActivationEvaluator.h>

namespace EFG::strct {
    distribution::factor::cnst::Factor mergeMessages(const Node& node) {
        std::set<const distribution::Distribution*> toMerge;
        gatherUnaries(toMerge, node);
        for (auto it = node.activeConnections.begin(); it != node.activeConnections.end(); ++it) {
            toMerge.emplace(it->second.message2This.get());
        }
        if (1 == toMerge.size()) {
            return distribution::factor::cnst::Factor(**toMerge.begin());
        }
        distribution::factor::modif::Factor factor(toMerge);
        factor.fillDomain();
        return factor;
    }

    std::vector<float> QueryHandler::getMarginalDistribution(const std::string& var) {
        auto varKey = categoric::makeVariable(2, var);
        auto itEv = this->evidences.find(varKey);
        if(itEv != this->evidences.end()) {
            // evidence
            return distribution::factor::cnst::IndicatorFactor(this->nodes.find(varKey)->second.variable, itEv->second).getProbabilities();
        }
        auto itN = this->nodes.find(varKey);
        if (itN == this->nodes.end()) {
            throw Error("non existent variable");
        }
        this->propagateBelief(PropagationKind::Sum);
        return mergeMessages(itN->second).getProbabilities();
    }

    distribution::factor::cnst::Factor QueryHandler::getJointMarginalDistribution(const std::set<std::string>& subgroup) {
        std::set<Node*> hiddenGroup;
        std::list<distribution::factor::cnst::IndicatorFactor> indicators;
        std::for_each(subgroup.begin(), subgroup.end(), [&](const std::string& name) {
            auto itN = this->nodes.find(categoric::makeVariable(2, name));
            if (itN == this->nodes.end()) {
                throw Error("non existent variable");
            }
            auto itOb = this->evidences.find(itN->first);
            if (itOb == this->evidences.end()) {
                // hidden
                hiddenGroup.emplace(&itN->second);
            }
            else {
                indicators.emplace_back(itN->first, itOb->second);
            }
        });

        this->propagateBelief(PropagationKind::Sum);

        // gather hidden variables factors
        std::set<const distribution::Distribution*> toMerge;
        {
            std::for_each(hiddenGroup.begin(), hiddenGroup.end(), [&toMerge, &hiddenGroup](const Node* n) {
                gatherUnaries(toMerge, *n);
                for (auto c = n->activeConnections.begin(); c != n->activeConnections.end(); ++c) {
                    if (hiddenGroup.find(c->first) == hiddenGroup.end()) {
                        // connection to node outside of the subgraph
                        toMerge.emplace(c->second.message2This.get());
                    }
                    else {
                        // connection to node inside of the subgraph
                        toMerge.emplace(c->second.factor.get());
                    }
                }
            });
        }
        std::for_each(indicators.begin(), indicators.end(), [&toMerge](const distribution::factor::cnst::IndicatorFactor& i) {
            toMerge.emplace(&i);
        });
        return distribution::factor::cnst::Factor(toMerge);
    }

    std::size_t getMAPnode(const Node& node) {
        auto iter = mergeMessages(node).getIterator();
        std::size_t maxPos = 0, pos = 0;
        float maxVal = iter.getImage(), val;
        ++iter;
        ++pos;
        iterator::forEach(iter, [&maxPos, &pos, &maxVal, &val](distribution::DistributionIterator& i) {
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
        auto varKey = categoric::makeVariable(2, var);
        auto itEv = this->evidences.find(varKey);
        if(itEv != this->evidences.end()) {
            // evidence
            return itEv->second;
        }
        auto itN = this->nodes.find(varKey);
        if (itN == this->nodes.end()) {
            throw Error("non existent variable");
        }
        this->propagateBelief(PropagationKind::MAP);
        return getMAPnode(itN->second);
    }

    std::vector<size_t> QueryHandler::getHiddenSetMAP() {
        this->propagateBelief(PropagationKind::MAP);
        auto hiddenVars = this->getHiddenVariables();
        std::vector<size_t> MAP;
        MAP.reserve(hiddenVars.size());
        std::for_each(hiddenVars.begin(), hiddenVars.end(), [this, &MAP](const categoric::VariablePtr& v) {
            MAP.push_back(getMAPnode(this->nodes.find(v)->second));
        });
        return MAP;
    }

    ActivationEvaluator QueryHandler::getActivationEvaluator() const {
        return ActivationEvaluator(this->getAllFactors(), this->getVariables());
    }
}
