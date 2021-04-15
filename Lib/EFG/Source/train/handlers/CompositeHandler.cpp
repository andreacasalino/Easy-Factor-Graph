/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <train/handlers/CompositeHandler.h>
#include <algorithm>

namespace EFG::train::handler {
    CompositeHandler::CompositeHandler(TrainHandlerPtr element) {
        this->addElement(std::move(element));
    }

    void CompositeHandler::setTrainSet(TrainSetPtr newSet, const std::set<categoric::VariablePtr>& modelVariables) {
        std::for_each(this->elements.begin(), this->elements.end(), [&](TrainHandlerPtr& h) {
            h->setTrainSet(newSet, modelVariables);
        });
    }

    float CompositeHandler::getGradientAlpha() {
        float grad = 0.f;
        std::for_each(this->elements.begin(), this->elements.end(), [&grad](TrainHandlerPtr& h) {
            grad += h->getGradientAlpha();
        });
        return grad;
    }

    float CompositeHandler::getGradientBeta() {
        float grad = 0.f;
        std::for_each(this->elements.begin(), this->elements.end(), [&grad](TrainHandlerPtr& h) {
            grad += h->getGradientBeta();
        });
        return grad;
    }

    void CompositeHandler::setWeight(const float& w) {
        std::for_each(this->elements.begin(), this->elements.end(), [&w](TrainHandlerPtr& h) {
            h->setWeight(w);
        });
    }
}
