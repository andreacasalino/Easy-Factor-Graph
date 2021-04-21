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
        TrainSetExtractor(TrainSetPtr completeTrainSet) : completeTrainSet(completeTrainSet) {};

        virtual TrainSetPtr getTrainSet() = 0;

        TrainSetPtr completeTrainSet;
    };

    class BasicExtractor : public TrainSetExtractor {
    protected:
        BasicExtractor(TrainSetPtr completeTrainSet) : TrainSetExtractor(completeTrainSet) {};

        inline TrainSetPtr getTrainSet() override { return this->completeTrainSet; };
    };

    class StochasticExtractor : public TrainSetExtractor {
    public:
        void setPercentage(const float& percentage);

    protected:
        StochasticExtractor(TrainSetPtr completeTrainSet) : TrainSetExtractor(completeTrainSet) {};

        inline TrainSetPtr getTrainSet() override { return std::make_shared<TrainSet>(this->completeTrainSet->getRandomSubSet(this->percentage)); };

    private:
        float percentage = 0.1f;
    };
}

#endif
