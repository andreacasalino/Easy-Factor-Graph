/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAIN_TRAINER_H
#define EFG_TRAIN_TRAINER_H

#include <train/Trainable.h>

namespace EFG::train {
    class TrainerBase {
    protected:
        TrainerBase() = default;

        virtual std::vector<float> getGradient() = 0;

        Trainable* model;
        TrainSetPtr trainSet = nullptr;
    };

    class Advancer : public virtual TrainerBase {
    protected:
        Advancer(train::Trainable& model, train::TrainSetPtr trainSet) {
            this->model = &model;
            this->trainSet = trainSet;
        };

        virtual void advance() = 0;
    };

    // full train set
    class BasicExtractor : public virtual TrainerBase {
    protected:
        std::vector<float> getGradient() final;
    };

    // stochastic extraction
    class StochasticExtractor : public virtual TrainerBase {
    protected:
        std::vector<float> getGradient() final;
    };

    // todo make ancestor templates
    template<typename AdvancerT, typename GradientExtractorT>
    class Trainer
        : public AdvancerT
        , public GradientExtractorT {
    public:
        static void train(train::Trainable& model, train::TrainSetPtr trainSet) {
            Trainer trainer(model, trainSet);
            // todo train model
        };

    protected:
        Trainer(train::Trainable& model, train::TrainSetPtr trainSet);
    };

    


    class GradientDescend {
    public:
        GradientDescend() = default;

        void train(train::Trainable& model, train::TrainSetPtr trainSet) const;

        void setAdvancement(float coeff) { this->advancementCoefficient = coeff; };
        void setMaxIterations(std::size_t iterations) { this->maxIterations = iterations; };

    private:
        float advancementCoefficient = 0.05f;
        std::size_t maxIterations = 1000;

    };
}

#endif
