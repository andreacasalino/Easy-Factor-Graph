/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAIN_HANDLER_HIDDEN_OBSERVED_H
#define EFG_TRAIN_HANDLER_HIDDEN_OBSERVED_H

#include <train/handlers/BaseHandler.h>
#include <structure/Node.h>

namespace EFG::train::handler {
    class HiddenObservedHandler : public BaseHandler {
    public:
        HiddenObservedHandler(strct::Node& nodeHidden, const std::pair<categoric::VariablePtr, const std::size_t*>& observed, std::shared_ptr<distribution::factor::modif::FactorExponential> factor);

        float getGradientBeta() final;

    protected:
        strct::Node* nodeHidden;
        categoric::VariablePtr observedVar;
        const std::size_t* observedVal;
    };
}

#endif
