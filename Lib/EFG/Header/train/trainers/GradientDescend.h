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
    template<typename Extractor>
    class GradientDescend 
        : public Trainer
        , public Extractor {
        static_assert(std::is_base_of<TrainSetExtractor , Extractor>::value, "Extractor should derive from TrainSetExtractor");
    public:
        GradientDescend() = default;

        void train(Trainable& model, TrainSetPtr trainSet) override {
            throw 0; // implement termination criteria
            std::vector<float> wGrad, w;
            for(std::size_t k=0; k<this->maxIterations; ++k) {
                wGrad = model.getGradient(this->getTrainSet());
                for(std::size_t j = 0; j<w.size(); ++j) {
                    w[j] += this->advancement * wGrad[j];
                }
                model.setWeights(w);
            }
        };

        float getAdvancement() const { return this->advancement; };
        void setAdvancement(float adv);

    private:
        float advancement = 0.1f;
    };
}

#endif
