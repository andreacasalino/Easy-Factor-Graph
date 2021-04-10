/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EmpiricalFrequencies.h>
#include <Error.h>
#include <algorithm>

namespace EFG::sample {
    std::size_t findPosition(const categoric::VariablePtr& var2Search, const categoric::Group& group) {
        auto it = group.getVariables().find(var2Search);
        if (it == group.getVariables().end()) {
            throw Error("variable not found");
        }
        return std::distance(group.getVariables().begin(), it);
    }

    std::vector<float> getEmpiricalMarginalFrequencies(categoric::VariablePtr var2Search, const std::vector<Combination>& samples, const categoric::Group& samplesGroup) {
        if (samples.empty()) {
            throw Error("samples container can't be empty");
        }
        std::size_t pos = findPosition(var2Search, samplesGroup);

        std::vector<float> frequencies(var2Search->size(), 0.f);
        std::for_each(samples.begin(), samples.end(), [&frequencies, &pos](const Combination& c) {
            ++frequencies[c.data()[pos]];
        });
        for (std::size_t k = 0; k < frequencies.size(); ++k) {
            frequencies[k] /= samples.size();
        }
        return frequencies;
    }

    float getEmpiricalFrequencies(const Combination& comb2Search, const categoric::Group& combGroup, const std::vector<Combination>& samples, const categoric::Group& samplesGroup) {
        if (samples.empty()) {
            throw Error("samples container can't be empty");
        }
        std::vector<std::size_t> posInSamples;
        posInSamples.reserve(combGroup.getVariables().size());
        std::for_each(combGroup.getVariables().begin(), combGroup.getVariables().end(), [&posInSamples, &samplesGroup](const categoric::VariablePtr& v) {
            posInSamples.push_back(findPosition(v , samplesGroup));
        });

        float freq = 0.f;
        std::for_each(samples.begin(), samples.end(), [&](const Combination& c) {
            for (std::size_t k = 0; k < posInSamples.size(); ++k) {
                if (c.data()[posInSamples[k]] != comb2Search.data()[k]) {
                    return;
                }
            }
            ++freq;
        });
        return freq /= samples.size();
    }
}
