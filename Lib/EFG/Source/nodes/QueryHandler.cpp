/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/QueryHandler.h>
#include <distribution/factor/modifiable/Factor.h>
#include <distribution/DistributionIterator.h>
#include "Commons.h"
#include <algorithm>
#include <Error.h>

namespace EFG::nodes {
    distribution::factor::cnst::Factor mergeMessages(const Node& node) {
        std::set<const distribution::Distribution*> toMerge;
        gatherUnaries(toMerge, node);
        for (auto it = node.activeConnections.begin(); it != node.activeConnections.end(); ++it) {
            toMerge.emplace(it->second.message2This.get());
        }
        if (toMerge.empty()) {
            distribution::factor::modif::Factor factor(categoric::Group(node.variable));
            factor.setImageEntireDomain(1.f);
            return factor;
        }
        distribution::factor::modif::Factor factor(toMerge);
        factor.emplaceEntireDomain();
        return factor;
    }

    std::vector<float> QueryHandler::getMarginalDistribution(const std::string& var) {
        auto itN = this->findNode(var);
        if (itN == this->nodes.end()) {
            throw Error("non existent variable");
        }
        if (nullptr == this->lastPropagation) {
            this->propagateBelief(PropagationKind::Sum);
        }
        return mergeMessages(itN->second).getProbabilities();
    }

    class IndicatorFactor : public distribution::factor::cnst::Factor {
    public:
        IndicatorFactor(categoric::VariablePtr var, std::size_t evidence)
            : Factor(categoric::Group(var)) {
            this->values->emplace(Combination({ evidence }), 1.f);
        };
    };
    distribution::factor::cnst::Factor QueryHandler::getJointMarginalDistribution(const std::vector<std::string>& subgroup) {
        std::set<Node*> subGraphSet;
        std::list<IndicatorFactor> indicators;
        std::for_each(subgroup.begin(), subgroup.end(), [&](const std::string& name) {
            auto itN = this->findNode(name);
            if (itN == this->nodes.end()) {
                throw Error("non existent variable");
            }
            auto itOb = this->evidences.find(itN->first);
            if (itOb == this->evidences.end()) {
                // hidden
                subGraphSet.emplace(&itN->second);
            }
            else {
                indicators.emplace_back(itN->first, itOb->second);
            }
        });

        if (nullptr == this->lastPropagation) {
            this->propagateBelief(PropagationKind::Sum);
        }

        std::set<const distribution::Distribution*> toMerge;
        std::for_each(indicators.begin(), indicators.end(), [&toMerge](const IndicatorFactor& i) {
            toMerge.emplace(&i);
        });
        std::for_each(subGraphSet.begin(), subGraphSet.end(), [&toMerge, &subGraphSet](const Node* n) {
            gatherUnaries(toMerge, *n);
            for (auto c = n->activeConnections.begin(); c != n->activeConnections.end(); ++c) {
                if (subGraphSet.find(c->first) == subGraphSet.end()) {
                    // connection to node outside of the subgraph
                    toMerge.emplace(c->second.message2This.get());
                }
                else {
                    // connection to node inside of the subgraph
                    toMerge.emplace(c->second.factor.get());
                }
            }
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
        auto itN = this->findNode(var);
        if (itN == this->nodes.end()) {
            throw Error("non existent variable");
        }
        if (nullptr == this->lastPropagation) {
            this->propagateBelief(PropagationKind::MAP);
        }
        return getMAPnode(itN->second);
    }

    std::vector<size_t> QueryHandler::getHiddenSetMAP() {
        if (nullptr == this->lastPropagation) {
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
