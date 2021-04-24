/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_GIBBS_SAMPLER_H
#define EFG_NODES_GIBBS_SAMPLER_H

#include <nodes/bases/EvidenceAware.h>
#include <nodes/bases/ThreadPoolAware.h>

namespace EFG::nodes {
    class GibbsSampler 
        : virtual public EvidenceAware
#ifdef THREAD_POOL_ENABLED
        , virtual public ThreadPoolAware
#endif
    {
    public:
        /**
         * @brief Use Gibbs sampling to draw samples for the hidden variables, assumgin the current evidences.
         * @param number of samples to draw
         * @param number of iterations used to evolve the model between the drawing of one sample and another
         */
        std::vector<Combination> getHiddenSetSamples(std::size_t numberOfSamples, std::size_t deltaIteration = 100) const;

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
        typedef std::map<categoric::VariablePtr, NodeHidden> HiddenStructure;

        HiddenStructure getHiddenStructure() const;

        class UniformSampler;
        static void evolveSamples(HiddenStructure& structure, std::size_t iterations, UniformSampler& sampler);

        static std::vector<std::size_t> convert(const HiddenStructure& structure);
    };
}

#endif
