/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <structure/components/StructureTunableAware.h>
#include <structure/components/StructureAware.h>
#include <Error.h>
#include <algorithm>

namespace EFG::strct {
    std::vector<std::vector<std::shared_ptr<distribution::factor::modif::FactorExponential>>> StructureTunableAware::getFactorsExp() const {
        std::vector<std::vector<std::shared_ptr<distribution::factor::modif::FactorExponential>>> result;
        result.resize(this->numberOfClusters);
        for (auto it = this->factorsExp.begin(); it != this->factorsExp.end(); ++it) {
            result[it->second].push_back(it->first);
        }
        return result;
    }

    float StructureTunableAware::findWeight(std::size_t cluster) const {
        for (auto it = this->factorsExp.begin(); it != this->factorsExp.end(); ++it) {
            if (cluster == it->second) {
                return it->first->getWeight();
            }
        }
        throw Error("inexistent cluster");
    }

    std::vector<float> StructureTunableAware::getWeights() const {
        std::vector<float> w;
        w.reserve(this->numberOfClusters);
        for (std::size_t k = 0; k < this->numberOfClusters; ++k) {
            w.push_back(this->findWeight(k));
        }
        return w;
    }
}
