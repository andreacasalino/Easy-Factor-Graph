/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/GibbsSampler.h>
#include <distribution/factor/modifiable/Factor.h>
#include "Commons.h"
#include <algorithm>
#include <random>

namespace EFG::nodes {
    class GibbsSampler::UniformSampler {
    public:
        UniformSampler() = default;

        inline float sample() const { return this->distribution(this->generator); };

        std::size_t sampleFromDiscrete(const std::vector<float>& distribution) const;

    private:
        mutable std::default_random_engine generator;
        mutable std::uniform_real_distribution<float> distribution;
    };

    std::size_t GibbsSampler::UniformSampler::sampleFromDiscrete(const std::vector<float>& distribution) const {
        float s = this->sample();
        float cumul = 0.f;
        for (std::size_t k = 0; k < distribution.size(); ++k) {
            cumul += distribution[k];
            if (s <= cumul) {
                return k;
            }
        }
        return distribution.size() - 1;
    }

    GibbsSampler::HiddenStructure GibbsSampler::getHiddenStructure() const {
        HiddenStructure structure;
        std::for_each(this->hidden.clusters.begin(), this->hidden.clusters.end(), [&structure](const std::set<Node*>& s) {
            std::for_each(s.begin(), s.end(), [&structure](const Node* n) {
                distribution::DistributionPtr unaryMerged;
                if (!n->unaryFactors.empty()) {
                    std::set<const distribution::Distribution*> toMerge;
                    gatherUnaries(toMerge, *n);
                    unaryMerged = std::make_shared<distribution::factor::cnst::Factor>(toMerge);
                }
                NodeHidden node{ 0, unaryMerged, {} };
                for (auto itC = n->activeConnections.begin(); itC != n->activeConnections.end(); ++itC) {
                    node.connections.push_back(NodeHiddenConnection{ itC->second.factor, itC->first->variable, nullptr });
                }
                structure.emplace(n->variable, node);
            });
        });
        for (auto itS = structure.begin(); itS != structure.end(); ++itS) {
            for (auto itC = itS->second.connections.begin(); itC != itS->second.connections.end(); ++itC) {
                itC->neighbourSample = &structure.find(itC->neighbourVariable)->second.sample;
            }
        }
        return structure;
    }

    void GibbsSampler::evolveSamples(HiddenStructure& structure, std::size_t iterations, UniformSampler& sampler) {
        std::set<const distribution::Distribution*> toMerge;
        for (std::size_t i = 0; i < iterations; ++i) {
            for (auto it = structure.begin(); it != structure.end(); ++it) {
                toMerge.clear();
                if (nullptr != it->second.unaryMerged) {
                    toMerge.emplace(it->second.unaryMerged.get());
                }
                std::list<distribution::factor::cnst::Factor> marginalized;
                for (auto c = it->second.connections.begin(); c != it->second.connections.end(); ++c) {
                    marginalized.emplace_back(*c->factor, Combination({ *c->neighbourSample }), categoric::Group(c->neighbourVariable));
                    toMerge.emplace(&marginalized.back());
                }
                if (toMerge.empty()) {
                    distribution::factor::modif::Factor temp(categoric::Group(it->first));
                    temp.setImageEntireDomain(1.f);
                    it->second.sample = sampler.sampleFromDiscrete(temp.getProbabilities());
                }
                else if (1 == toMerge.size()) {
                    it->second.sample = sampler.sampleFromDiscrete((*toMerge.begin())->getProbabilities());
                }
                else {
                    it->second.sample = sampler.sampleFromDiscrete(distribution::factor::cnst::Factor(toMerge).getProbabilities());
                }
            }
        }
    }

    std::vector<std::size_t> GibbsSampler::convert(const HiddenStructure& structure) {
        std::vector<std::size_t> converted;
        converted.reserve(structure.size());
        for (auto it = structure.begin(); it != structure.end(); ++it) {
            converted.push_back(it->second.sample);
        }
        return converted;
    }

    std::vector<Combination> GibbsSampler::getHiddenSetSamples(std::size_t numberOfSamples, std::size_t deltaIteration) const {
        UniformSampler sampler;
        auto structure = this->getHiddenStructure();
        std::vector<Combination> samples;
        samples.reserve(numberOfSamples);
        evolveSamples(structure, 10 * deltaIteration, sampler);
        for (std::size_t s = 0; s < numberOfSamples; ++s) {
            evolveSamples(structure, deltaIteration, sampler);
            samples.push_back(convert(structure));
        }
        return samples;
    }
}
