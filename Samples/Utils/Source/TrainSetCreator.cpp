/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <TrainSetCreator.h>
#include <algorithm>

namespace EFG::sample {
    std::vector<std::size_t> findPositions(const std::set<categoric::VariablePtr>& set, const std::set<categoric::VariablePtr>& subset) {
        std::vector<std::size_t> pos;
        pos.reserve(subset.size());
        std::for_each(subset.begin(), subset.end(), [&](const categoric::VariablePtr& v) {
            pos.push_back(std::distance(set.begin(), set.find(v)));
        });
        return pos;
    };

    TrainSetCreator::TrainSetCreator(model::ConditionalRandomField& model)
        : posObs(findPositions(model.getVariables(), model.getObservedVariables()))
        , posHidden(findPositions(model.getVariables(), model.getHiddenVariables()))
        , model(&model) {
    }

    std::vector<categoric::Combination> TrainSetCreator::getSamples(const std::vector<std::size_t>& evidences, std::size_t numberOfSamples, std::size_t deltaIteration) {
        std::vector<std::size_t> comb(this->posHidden.size() + this->posObs.size(), 0);
        this->model->setEvidences(evidences);
        for (std::size_t p = 0; p < this->posObs.size(); ++p) {
            comb[this->posObs[p]] = evidences[p];
        }
        std::vector<categoric::Combination> samples;
        samples.reserve(numberOfSamples);
        auto hiddenSamples = this->model->getHiddenSetSamples(numberOfSamples, deltaIteration);
        for (std::size_t k = 0; k < hiddenSamples.size(); ++k) {
            for (std::size_t p = 0; p < this->posHidden.size(); ++p) {
                comb[this->posHidden[p]] = hiddenSamples[k].data()[p];
            }
            samples.emplace_back(comb.data(), comb.size());
        }
        return samples;
    }
}
