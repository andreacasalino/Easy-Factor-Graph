/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_GIBBS_SAMPLER_H
#define EFG_NODES_GIBBS_SAMPLER_H

#include <nodes/NodesContainer.h>

namespace EFG::nodes {
    class GibbsSampler : virtual public NodesContainer {
    public:
        std::vector<std::vector<std::size_t>> getHiddenSetSamples(std::size_t numberOfSamples, std::size_t deltaIteration = 100) const;

    private:
        struct NodeHidden;
        struct NodeHiddenConnection {
            distribution::DistributionPtr factor;
            categoric::VariablePtr neighbourVariable;
            std::size_t* neighbourSample;
        };
        struct NodeHidden {
            std::size_t sample;
            distribution::DistributionPtr unaryMerged;
            std::list<NodeHiddenConnection> connections;
        };
        typedef std::map<categoric::VariablePtr, NodeHidden> SamplesStructure;

        SamplesStructure getSamplesStructure() const;

        class UniformSampler;
        static void evolveSamples(SamplesStructure& structure, std::size_t iterations, UniformSampler& sampler);

        static std::vector<std::size_t> convert(const SamplesStructure& structure);
    };
}

#endif
