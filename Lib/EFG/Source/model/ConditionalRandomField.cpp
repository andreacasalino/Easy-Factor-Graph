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

namespace EFG::model {
    void ConditionalRandomField::Insert(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert) {
        this->InsertTunableCapable::Insert(toInsert);
        this->insertHandler(toInsert.get());
    }

    void ConditionalRandomField::Insert(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert, const categoric::Group& potentialSharingWeight) {
        this->InsertTunableCapable::Insert(toInsert, potentialSharingWeight);
        this->insertHandler(toInsert.get());
    }

    train::TrainHandlerPtr ConditionalRandomField::makeHandler(distribution::factor::modif::FactorExponential* factor) {
        auto handler = this->Trainable::makeHandler(factor);
        if (nullptr != dynamic_cast<train::handler::UnaryHandler*>(handler.get())) {
            if (this->evidences.find(*factor->getGroup().getVariables().begin()) != this->evidences.end()) {
                throw Error("tuanble factor attached to node observed is invalid");
            }
        }
        if (nullptr != dynamic_cast<train::handler::BinaryHandler*>(handler.get())) {
            auto itOa = this->evidences.find(*factor->getGroup().getVariables().begin());
            auto itOb = this->evidences.find(*factor->getGroup().getVariables().rbegin());
            if ((itOa == this->evidences.end()) && (itOb == this->evidences.end())) {
                throw Error("tuanble factor connectioning 2 observartions is invalid");
            }
            if (itOa != this->evidences.end()) {
                handler = std::make_unique<train::handler::HiddenObservedHandler>(this->nodes.find(itOb->first), std::make_pair(itOa->first, itOa->second), nodes::convert(factor));
            }
            if (itOb != this->evidences.end()) {
                handler = std::make_unique<train::handler::HiddenObservedHandler>(this->nodes.find(itOa->first), std::make_pair(itOb->first, itOb->second), nodes::convert(factor));
            }
        }
        return handler;
    }
}
