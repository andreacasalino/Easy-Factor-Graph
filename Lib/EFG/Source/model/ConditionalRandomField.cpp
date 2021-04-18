/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <model/ConditionalRandomField.h>
#include <train/handlers/BinaryHandler.h>
#include <train/handlers/UnaryHandler.h>
#include "HiddenObservedHandler.h"
#include <Error.h>
#include <algorithm>

namespace EFG::model {
    void ConditionalRandomField::Insert(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert) {
        this->InsertTunableCapable::Insert(toInsert);
        this->insertHandler(toInsert);
    }

    void ConditionalRandomField::Insert(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert, const categoric::Group& potentialSharingWeight) {
        this->InsertTunableCapable::Insert(toInsert, potentialSharingWeight);
        this->insertHandler(toInsert);
    }

    train::TrainHandlerPtr ConditionalRandomField::makeHandler(std::shared_ptr<distribution::factor::modif::FactorExponential> factor) {
        auto handler = this->Trainable::makeHandler(factor);
        if (nullptr != dynamic_cast<train::handler::UnaryHandler*>(handler.get())) {
            if (this->evidences.find(*factor->getGroup().getVariables().begin()) != this->evidences.end()) {
                throw Error("tuanble factor attached to node observed is invalid");
            }
        }
        if (nullptr != dynamic_cast<train::handler::BinaryHandler*>(handler.get())) {
            auto itOa = this->evidences.find(*factor->getGroup().getVariables().begin());
            auto itOb = this->evidences.find(*factor->getGroup().getVariables().rbegin());
            if ((itOa != this->evidences.end()) && (itOb != this->evidences.end())) {
                throw Error("tuanble factor connectioning 2 observartions is invalid");
            }
            if (itOa != this->evidences.end()) {
                handler = std::make_unique<train::handler::HiddenObservedHandler>(this->nodes.find(itOb->first)->second, std::make_pair(itOa->first, &itOa->second), factor);
            }
            if (itOb != this->evidences.end()) {
                handler = std::make_unique<train::handler::HiddenObservedHandler>(this->nodes.find(itOa->first)->second, std::make_pair(itOb->first, &itOb->second), factor);
            }
        }
        return handler;
    }

    std::vector<float> ConditionalRandomField::getGradient() {
        this->lastPropagation.reset();
        std::vector<float> grad;
        grad.resize(this->handlers.size());
        std::size_t pos = 0;
        // compute alpha part
        std::for_each(this->handlers.begin(), this->handlers.end(), [&grad, &pos](train::TrainHandlerPtr& h) {
            grad[pos] = h->getGradientAlpha();
            ++pos;
        });
        // compute beta part
        auto trainSet = this->getTrainSet();
        std::vector<std::size_t> observationPositions;
        observationPositions.reserve(this->evidences.size());
        auto vars = this->getVariables();
        for (auto it = this->evidences.begin(); it != this->evidences.end(); ++it) {
            observationPositions.push_back(std::distance(vars.begin(), vars.find(it->first)));
        }
        std::vector<std::size_t> observations;
        observations.resize(observationPositions.size());
        float coeff = 1.f / static_cast<float>(trainSet->getSet().size());
        auto setObservations = [&](const Combination& comb){
            for (std::size_t k = 0; k < observationPositions.size(); ++k) {
                observations[k] = comb.data()[observationPositions[k]];
            }
            this->setEvidences(observations);
            this->propagateBelief(nodes::PropagationKind::Sum);
        };
#ifdef THREAD_POOL_ENABLED
        if (nullptr != this->threadPool) {
            std::for_each(trainSet->getSet().begin(), trainSet->getSet().end(), [&](const Combination& comb) {
                setObservations(comb);
                pos = 0;
                std::for_each(this->handlers.begin(), this->handlers.end(), [&](train::TrainHandlerPtr& h) {
                    train::TrainHandler* pt = h.get();
                    this->threadPool->push([pt, pos, &grad, &coeff]() { grad[pos] -= coeff * pt->getGradientBeta(); });
                    ++pos;
                });
                this->threadPool->wait();
            });
        }
        else {
#endif
            std::for_each(trainSet->getSet().begin(), trainSet->getSet().end(), [&](const Combination& comb) {
                setObservations(comb);
                pos = 0;
                std::for_each(this->handlers.begin(), this->handlers.end(), [&](train::TrainHandlerPtr& h) {
                    grad[pos] -= coeff * h->getGradientBeta();
                    ++pos;
                });
            });
#ifdef THREAD_POOL_ENABLED
        }
#endif
        return grad;
    }
}
