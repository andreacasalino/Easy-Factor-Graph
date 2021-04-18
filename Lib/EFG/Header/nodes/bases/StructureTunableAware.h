/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_STRUCTURE_TUNABLE_AWARE_H
#define EFG_NODES_STRUCTURE_TUNABLE_AWARE_H

#include <nodes/bases/Base.h>
#include <distribution/factor/modifiable/FactorExponential.h>

namespace EFG::nodes {
    class StructureTunableAware : virtual public Base {
    public:
        std::vector<std::vector<std::shared_ptr<distribution::factor::modif::FactorExponential>>> getFactorsTunable() const;

        std::vector<float> getWeights() const;

    protected:
        std::size_t numberOfClusters = 0;
        // factor, cluster id
        std::map<std::shared_ptr<distribution::factor::modif::FactorExponential>, std::size_t> factorsTunable;

    private:
        float findWeight(std::size_t cluster) const;
    };
}

#endif
