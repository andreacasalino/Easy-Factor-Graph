/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include "HiddenObservedHandler.h"
#include <distribution/factor/modifiable/Factor.h>
#include <distribution/factor/const/Indicator.h>
#include "../../structure/Commons.h"

namespace EFG::train::handler {
    HiddenObservedHandler::HiddenObservedHandler(strct::Node& nodeHidden, const std::pair<categoric::VariablePtr, const std::size_t*>& observed, std::shared_ptr<distribution::factor::modif::FactorExponential> factor)
        : BaseHandler(factor)
        , nodeHidden(&nodeHidden) {
        this->observedVar = observed.first;
        this->observedVal = observed.second;
    }

    float HiddenObservedHandler::getGradientBeta() {
        float beta = 0.f;
        std::set<const distribution::Distribution*> toMerge;
        strct::gatherUnaries(toMerge, *this->nodeHidden);
        for (auto it = this->nodeHidden->activeConnections.begin(); it != this->nodeHidden->activeConnections.end(); ++it) {
            toMerge.emplace(it->second.message2This.get());
        }
        distribution::factor::cnst::IndicatorFactor Indicator(this->observedVar, *this->observedVal);
        toMerge.emplace(&Indicator);
        return this->dotProduct(distribution::factor::cnst::Factor(toMerge).getProbabilities());
    }
}
