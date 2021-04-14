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
    void BeliefPropagator::propagateBelief(const PropagationKind& kind) {
        this->lastPropagation = std::make_unique<PropagationResult>();
        this->lastPropagation->kindDone = kind;
#ifdef THREAD_POOL_ENABLED
        if (nullptr != this->threadPool) {
            std::for_each(this->hidden.clusters.begin(), this->hidden.clusters.end(), [&](const std::set<Node*>& cluster) {
                if ((!this->messagePassingThreadPool(cluster, kind)) && (!this->loopyPropagationThreadPool(cluster, kind))) {
                    this->lastPropagation->wasTerminated = false;
                }
                this->lastPropagation->wasTerminated = true;
            });
        }
        else {
#endif
            std::for_each(this->hidden.clusters.begin(), this->hidden.clusters.end(), [&](const std::set<Node*>& cluster) {
                if ((!this->messagePassing(cluster, kind)) && (!this->loopyPropagation(cluster, kind))) {
                    this->lastPropagation->wasTerminated = false;
                }
                this->lastPropagation->wasTerminated = true;
            });
#ifdef THREAD_POOL_ENABLED
        }
#endif
    }

    class MessageComputer {
    public:
        MessageComputer(Node* sender, const Connection* receiver, const PropagationKind& kind)
            : sender(sender)
            , receiver(receiver)
            , kind(kind) {
            this->toMerge = { receiver->factor.get() };
            gatherUnaries(toMerge, *sender);
            for (auto it = sender->activeConnections.begin(); it != sender->activeConnections.end(); ++it) {
                toMerge.emplace(it->second.message2This.get());
            }
            toMerge.erase(toMerge.find(receiver->message2This.get()));
        };
        MessageComputer(const MessageComputer& ) = default;

        inline bool isComputationPossible() const { return (this->toMerge.find(nullptr) == this->toMerge.end()); };
        // return difference
        float compute() {
            std::unique_ptr<distribution::Distribution> newMessage;
            if (PropagationKind::Sum == kind) {
                newMessage = std::make_unique<distribution::factor::cnst::MessageSum>(distribution::factor::cnst::Factor(toMerge), categoric::Group(sender->variable));
            }
            else {
                newMessage = std::make_unique<distribution::factor::cnst::MessageMAP>(distribution::factor::cnst::Factor(toMerge), categoric::Group(sender->variable));
            }
            if (nullptr == receiver->twin->message2This) {
                return MAX_DIFF;
            }
            float difference = 0.f;
            auto itOld = receiver->twin->message2This->getIterator();
            auto itNew = newMessage->getIterator();
            if (itOld.getNumberOfValues() != itNew.getNumberOfValues()) {
                difference = MAX_DIFF;
            }
            else {
                iterator::forEach(itOld, [&itNew, &difference](const distribution::DistributionIterator& itOld) {
                    difference += fabsf(itNew.getImageRaw() - itOld.getImageRaw());
                });
            }
            receiver->twin->message2This = std::move(newMessage);
            return difference;
        };

        inline const std::set<const distribution::Distribution*>& getDependencies() const { return this->toMerge; };

    private:
        Node* sender;
        const Connection* receiver;
        PropagationKind kind;
        std::set<const distribution::Distribution*> toMerge;
    };

    std::list<std::pair<Node*, const Connection*>> getOpenSet(const std::set<Node*>& cluster) {
        std::list<std::pair<Node*, const Connection*>> openSet;
        std::for_each(cluster.begin(), cluster.end(), [&openSet](Node* n) {
            for (auto itA = n->activeConnections.begin(); itA != n->activeConnections.end(); ++itA) {
                openSet.emplace_back(std::make_pair(n, &itA->second));
            }
        });
        return openSet;
    }

    bool BeliefPropagator::messagePassing(const std::set<Node*>& cluster, const PropagationKind& kind) {
        std::list<std::pair<Node*, const Connection*>> openSet = getOpenSet(cluster);
        bool progressWasMade;
        while (!openSet.empty()) {
            progressWasMade = false;
            auto it = openSet.begin();
            while (it != openSet.end()) {
                MessageComputer computer(it->first, it->second, kind);
                if (computer.isComputationPossible()) {
                    computer.compute();
                    it = openSet.erase(it);
                    progressWasMade = true;
                }
                else ++it;
            }
            if (!progressWasMade) {
                return false;
            }
        }
        return true;
    }

    void setOnesMessages(const std::set<Node*>& cluster) {
        std::for_each(cluster.begin(), cluster.end(), [](Node* n) {
            for (auto itA = n->activeConnections.begin(); itA != n->activeConnections.end(); ++itA) {
                auto mexOnes = std::make_unique<distribution::factor::modif::Factor>(itA->first->variable);
                mexOnes->setImageEntireDomain(1.f);
                itA->second.message2This = std::move(mexOnes);
            }
        });
    }

    bool BeliefPropagator::loopyPropagation(const std::set<Node*>& cluster, const PropagationKind& kind) {
        // set to ones all messages
        setOnesMessages(cluster);
        // calibrate messages
        float variationMax, variation;
        for (std::size_t k = 0; k < this->maxIterationsLoopyPropagtion; ++k) {
            variationMax = 0.f;
            std::for_each(cluster.begin(), cluster.end(), [&](Node* n) {
                for (auto itA = n->activeConnections.begin(); itA != n->activeConnections.end(); ++itA) {
                    variation = MessageComputer(itA->first, &itA->second, kind).compute();
                    if (variation > variationMax) {
                        variationMax = variation;
                    }
                }
            });
            if (0.f == variationMax) {
                return true;
            }
        }
        return false;
    }

#ifdef THREAD_POOL_ENABLED
    bool BeliefPropagator::messagePassingThreadPool(const std::set<Node*>& cluster, const PropagationKind& kind) {
        std::list<std::pair<Node*, const Connection*>> openSet = getOpenSet(cluster);
        bool progressWasMade;
        while (!openSet.empty()) {
            progressWasMade = false;
            auto it = openSet.begin();
            std::list<MessageComputer> computers;
            while (it != openSet.end()) {
                computers.emplace_back(it->first, it->second, kind);
                MessageComputer* cmpPtr = &computers.back();
                if (cmpPtr->isComputationPossible()) {
                    this->threadPool->push([cmpPtr]() {
                        cmpPtr->compute();
                    });
                    it = openSet.erase(it);
                    progressWasMade = true;
                }
                else ++it;
            }
            this->threadPool->wait();
            if (!progressWasMade) {
                return false;
            }
        }
        return true;
    }

    bool BeliefPropagator::loopyPropagationThreadPool(const std::set<Node*>& cluster, const PropagationKind& kind) {
        // set to ones all messages
        setOnesMessages(cluster);
        // get complete list of messages
        std::list<std::pair<Node*, const Connection*>> toCalibrate = getOpenSet(cluster);
        // calibrate messages
        std::atomic<float> variationMax;
        for (std::size_t k = 0; k < this->maxIterationsLoopyPropagtion; ++k) {
            variationMax = 0.f;
            std::list<std::pair<Node*, const Connection*>> openSet = toCalibrate;
            std::set<const distribution::Distribution*> calibrating;
            while (!openSet.empty()) {
                std::list<MessageComputer> computers;
                auto itOp = openSet.begin();
                while (itOp != openSet.end()) {
                    computers.emplace_back(itOp->first, itOp->second, kind);
                    MessageComputer* cmpPtr = &computers.back();
                    bool calibrationPossible = true;
                    // check that all dependencies will be not calibrated during this iteration
                    for (auto dep = cmpPtr->getDependencies().begin(); dep != cmpPtr->getDependencies().end(); ++dep) {
                        if (calibrating.find(*dep) != calibrating.end()) {
                            calibrationPossible = false;
                            break;
                        }
                    }
                    if (calibrationPossible) {
                        itOp = openSet.erase(itOp);
                        calibrating.emplace(itOp->second->twin->message2This.get());
                        this->threadPool->push([cmpPtr, &variationMax]() {
                            float variation = cmpPtr->compute();
                            if (variation > variationMax) {
                                variationMax = variation;
                            }
                        });
                    }
                    else {
                        computers.pop_back();
                        ++itOp;
                    }
                }
                this->threadPool->wait();
            }
            if (0.f == variationMax) {
                return true;
            }
        }
        return false;
    }
#endif
}
