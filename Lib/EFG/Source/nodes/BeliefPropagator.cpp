/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/BeliefPropagator.h>
#include <distribution/factor/const/Message.h>
#include <distribution/factor/modifiable/Factor.h>
#include <distribution/DistributionIterator.h>
#include "Commons.h"
#include <limits>
#include <algorithm>

constexpr float MAX_DIFF = std::numeric_limits<float>::max();

namespace EFG::nodes {
    struct MessageRecomputation {
        bool wasComputed = false;
        float difference = 0.f;
    };
    MessageRecomputation recomputeMessage(Node* sender, const Connection& receiver, const PropagationKind& kind) {
        MessageRecomputation info;
        std::set<const distribution::Distribution*> toMerge = { receiver.factor.get() };
        gatherUnaries(toMerge, *sender);
        for (auto it = sender->activeConnections.begin(); it != sender->activeConnections.end(); ++it) {
            toMerge.emplace(it->second.message2This.get());
        }
        toMerge.erase(toMerge.find(receiver.message2This.get()));
        if (toMerge.find(nullptr) != toMerge.end()) {
            return info;
        }
        std::unique_ptr<distribution::Distribution> newMessage;
        if (PropagationKind::Sum == kind) {
            newMessage = std::make_unique<distribution::factor::cnst::MessageSum>(distribution::factor::cnst::Factor(toMerge), categoric::Group(sender->variable));
        }
        else {
            newMessage = std::make_unique<distribution::factor::cnst::MessageMAP>(distribution::factor::cnst::Factor(toMerge), categoric::Group(sender->variable));
        }
        // evaluate difference
        info.wasComputed = true;
        if (nullptr == receiver.twin->message2This) {
            info.difference = MAX_DIFF;
        }
        else {
            auto itOld = receiver.twin->message2This->getIterator();
            auto itNew = newMessage->getIterator();
            if (itOld.getNumberOfValues() != itNew.getNumberOfValues()) {
                info.difference = MAX_DIFF;
            }
            else {
                iterator::forEach(itOld, [&itNew, &info](const distribution::DistributionIterator& itOld) {
                    info.difference += fabsf(itNew.getImageRaw() - itOld.getImageRaw());
                });
            }
        }
        receiver.twin->message2This = std::move(newMessage);
        return info;
    };

    void BeliefPropagator::propagateBelief(const PropagationKind& kind) {
        this->lastPropagation = std::make_unique<PropagationResult>();
        this->lastPropagation->kindDone = kind;
        this->lastPropagation->iterationsRequired = 0;
        std::for_each(this->hidden.clusters.begin(), this->hidden.clusters.end(), [&](const std::set<Node*>& cluster) {
            if ((!this->messagePassing(cluster, kind)) && (!this->loopyPropagation(cluster, kind))) {
                this->lastPropagation->wasTerminated = false;
            }
            this->lastPropagation->wasTerminated = true;
        });
    }

    bool BeliefPropagator::messagePassing(const std::set<Node*>& cluster, const PropagationKind& kind) {
        std::list<std::pair<Node*, const Connection*>> openSet;
        std::for_each(cluster.begin(), cluster.end(), [&openSet](Node* n) {
            for (auto itA = n->activeConnections.begin(); itA != n->activeConnections.end(); ++itA) {
                openSet.emplace_back(std::make_pair(n, &itA->second));
            }
        });
        bool progressWasMade;
        while (!openSet.empty()) {
            progressWasMade = false;
            auto it = openSet.begin();
            while (it != openSet.end()) {
                if (recomputeMessage(it->first, *it->second, kind).wasComputed) {
                    it = openSet.erase(it);
                    progressWasMade = true;
                }
                else ++it;
            }
            if (!progressWasMade) {
                return false;
            }
            ++this->lastPropagation->iterationsRequired;
        }
        return true;
    }

    bool BeliefPropagator::loopyPropagation(const std::set<Node*>& cluster, const PropagationKind& kind) {
        // set to ones all messages
        std::for_each(cluster.begin(), cluster.end(), [](Node* n) {
            for (auto itA = n->activeConnections.begin(); itA != n->activeConnections.end(); ++itA) {
                auto mexOnes = std::make_unique<distribution::factor::modif::Factor>(itA->first->variable);
                mexOnes->setImageEntireDomain(1.f);
                itA->second.message2This = std::move(mexOnes);
            }
        });
        // calibrate messages
        float variationMax, variation;
        for (std::size_t k = 0; k < this->maxIterationsLoopyPropagtion; ++k) {
            variationMax = 0.f;
            std::for_each(cluster.begin(), cluster.end(), [&](Node* n) {
                for (auto itA = n->activeConnections.begin(); itA != n->activeConnections.end(); ++itA) {
                    variation = recomputeMessage(itA->first, itA->second, kind).difference;
                    if (variation > variationMax) {
                        variationMax = variation;
                    }
                }
            });
            if (0.f == variationMax) {
                this->lastPropagation->iterationsRequired += k;
                return true;
            }
        }
        this->lastPropagation->iterationsRequired += this->maxIterationsLoopyPropagtion;
        return false;
    }
}
