/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <train/Trainable.h>
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
}
