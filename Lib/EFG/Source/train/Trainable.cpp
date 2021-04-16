/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <train/Trainable.h>
#include <train/handlers/UnaryHandler.h>
#include <train/handlers/BinaryHandler.h>
#include <nodes/bases/StructureAware.h>
#include <train/handlers/CompositeHandler.h>
#include <algorithm>
#include <Error.h>

namespace EFG::train {
    void Trainable::setTrainSet(TrainSetPtr newSet) {
        if (newSet->getSet().begin()->size() != this->nodes.size()) {
            throw Error("invalid train set");
        }
        auto vars = this->getVariables();
        std::for_each(this->handlers.begin(), this->handlers.end(), [&newSet, &vars](TrainHandlerPtr& h) {
            h->setTrainSet(newSet, vars);
        });
        this->set = newSet;
    }

    void Trainable::setWeights(const std::vector<float>& w) {
        if (w.size() != this->handlers.size()) {
            throw Error("invalid weigths size");
        }
        auto itW = w.begin();
        std::for_each(this->handlers.begin(), this->handlers.end(), [&itW](TrainHandlerPtr& h) {
            h->setWeight(*itW);
            ++itW;
        });
    };

    TrainHandlerPtr Trainable::makeHandler(distribution::factor::modif::FactorExponential* factor) {
        const auto& variables = factor->getGroup().getVariables();
        if (1 == variables.size()) {
            return std::make_unique<train::handler::UnaryHandler>(&this->nodes.find(*variables.begin())->second, nodes::convert(factor));
        }
        return std::make_unique<train::handler::BinaryHandler>(&this->nodes.find(*variables.begin())->second, &this->nodes.find(*variables.rbegin())->second, nodes::convert(factor));
    }

    void Trainable::insertHandler(distribution::factor::modif::FactorExponential* factor) {
        auto newHandler = this->makeHandler(factor);
        auto itF = this->factorsTunable.find(factor);
        if (this->handlers.size() == itF->second) {
            // it is a new cluster
            this->handlers.emplace_back(std::move(newHandler));
        }
        else {
            // should be added into a composite
            auto itHndl = this->handlers.begin();
            std::advance(itHndl, itF->second);
            train::handler::CompositeHandler* compositeIntrf = dynamic_cast<train::handler::CompositeHandler*>(itHndl->get());
            if (nullptr == compositeIntrf) {
                *itHndl = std::make_unique<train::handler::CompositeHandler>(std::move(*itHndl), std::move(newHandler));
            }
            else {
                compositeIntrf->addElement(std::move(newHandler));
            }
        }
    }
}
