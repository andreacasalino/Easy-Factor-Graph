/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_TRAIN_TRAIN_SET_EXTRACTOR_H
#define EFG_TRAIN_TRAIN_SET_EXTRACTOR_H

#include <train/TrainSet.h>

namespace EFG::train {
    class TrainSetExtractor {
    protected:
        TrainSetExtractor() = default;

        virtual TrainSetPtr getTrainSet() = 0;

        inline void setCompleteTrainSet(TrainSetPtr trainSet) { this->completeTrainSet = trainSet; };
        inline TrainSetPtr getCompleteTrainSet() { return this->completeTrainSet; };

    private:
        TrainSetPtr completeTrainSet;
    };

    class BasicExtractor : public TrainSetExtractor {
    protected:
        inline TrainSetPtr getTrainSet() override { return this->getCompleteTrainSet(); };
    };

    class StochasticExtractor : public TrainSetExtractor {
    public:
        void setPercentage(const float& percentage) { this->percentage = percentage; };

    protected:
        inline TrainSetPtr getTrainSet() override { return std::make_shared<TrainSet>(this->getCompleteTrainSet()->getRandomSubSet(this->percentage)); };

    private:
        float percentage = 0.1f;
    };
}

#endif
