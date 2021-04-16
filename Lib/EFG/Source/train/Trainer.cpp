/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <train/Trainer.h>

namespace EFG::train {
    inline void scale(std::vector<float>& v, float coeff) {
        for (std::size_t k = 0; k < v.size(); ++k) {
            v[k] *= coeff;
        }
    };

    inline void add(std::vector<float>& recipient, const std::vector<float>& toAdd) {
        for (std::size_t k = 0; k < toAdd.size(); ++k) {
            recipient[k] += toAdd[k];
        }
    };

    void GradientDescend::train(train::Trainable& model, train::TrainSetPtr trainSet) const {
        model.setTrainSet(trainSet);
        auto w = model.getWeights();
        std::vector<float> gradient;
        for (std::size_t k = 0; k < this->maxIterations; ++k) {
            gradient = model.getGradient();
            scale(gradient, this->advancementCoefficient);
            add(w, gradient);
            model.setWeights(w);
        }
    }
}
