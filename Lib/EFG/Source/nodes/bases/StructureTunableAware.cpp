/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/bases/StructureTunableAware.h>
#include <nodes/bases/StructureAware.h>
#include <Error.h>
#include <algorithm>

namespace EFG::nodes {
    std::vector<std::vector<std::shared_ptr<distribution::factor::modif::FactorExponential>>> StructureTunableAware::getFactorsTunable() const {
        std::vector<std::vector<std::shared_ptr<distribution::factor::modif::FactorExponential>>> result;
        result.resize(this->numberOfClusters);
        for (auto it = this->factorsTunable.begin(); it != this->factorsTunable.end(); ++it) {
            result[it->second].push_back(convert(it->first));
        }
        return result;
    }

    float StructureTunableAware::findWeight(std::size_t cluster) const {
        for (auto it = this->factorsTunable.begin(); it != this->factorsTunable.end(); ++it) {
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
