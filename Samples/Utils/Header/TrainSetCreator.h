/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_SAMPLE_TRAINSET_CREATOR_H
#define EFG_SAMPLE_TRAINSET_CREATOR_H

#include <model/ConditionalRandomField.h>

namespace EFG::sample {
    class TrainSetCreator {
    public:
        TrainSetCreator(model::ConditionalRandomField& model);

        std::vector<Combination> getSamples(const std::vector<std::size_t>& evidences, std::size_t numberOfSamples, std::size_t deltaIteration);

    private:
        const std::vector<std::size_t> posObs;
        const std::vector<std::size_t> posHidden;
        model::ConditionalRandomField* model;
    };
}

#endif
