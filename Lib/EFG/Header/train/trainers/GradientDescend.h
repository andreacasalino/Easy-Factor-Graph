/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAIN_GRADIENT_DESCEND_H
#define EFG_TRAIN_GRADIENT_DESCEND_H

#include <train/Trainer.h>
#include <train/trainers/TrainSetExtractor.h>

namespace EFG::train {
    template<typename Extractor = BasicExtractor>
    class GradientDescend : public Trainer {
    public:
        GradientDescend() = default;

        void train(Trainable& model, TrainSetPtr trainSet) override {
            throw 0; // implement termination criteria
            this->extractor = std::make_unique<Extractor>(trainSet);
            std::vector<float> wGrad, w;
            for(std::size_t k=0; k<this->maxIterations; ++k) {
                wGrad = model.getGradient(this->extractor->getTrainSet());
                for(std::size_t j = 0; j<w.size(); ++j) {
                    w[j] += this->advancement * wGrad[j];
                }
                model.setWeights(w);
            }
        };

        float getAdvancement() const { return this->advancement; };
        void setAdvancement(float adv) { this->advancement = adv; };

    private:
        float advancement = 0.1f;
        std::unique_ptr<Extractor> extractor;
    };
}

#endif
