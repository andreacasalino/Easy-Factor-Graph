/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/factor/const/Message.h>
#include <distribution/DistributionIterator.h>
#include <categoric/Range.h>
#include <Error.h>
#include <algorithm>
#include <list>

namespace EFG::distribution::factor::cnst {
    std::vector<std::size_t> getSubsetIndices(const categoric::Group& set, const categoric::Group& subset) {
        std::vector<std::size_t> indexSubset;
        indexSubset.reserve(subset.getVariables().size());
        std::for_each(subset.getVariables().begin(), subset.getVariables().end(), [&indexSubset, &set](const categoric::VariablePtr& v) {
            indexSubset.push_back(std::distance(set.getVariables().begin(), set.getVariables().find(v)));
        });
        return indexSubset;
    }

    template <typename ValueComputer>
    void messageBuilder(std::map<Combination, float>& recipient, const distribution::Distribution& factor, const categoric::Group& remaining, ValueComputer computer) {
        auto index = getSubsetIndices(factor.getGroup(), remaining);
        std::list<float> values;
        categoric::Range range(remaining);
        iterator::forEach(range, [&](const categoric::Range& r) {
            values.clear();
            auto iter = factor.getIterator();
            iterator::forEach(iter, [&](const distribution::DistributionIterator& iter) {
                for (std::size_t k = 0; k < r.get().size(); ++k) {
                    if (r.get()[k] != iter.getCombination().data()[index[k]]) {
                        return;
                    }
                    values.push_back(iter.getImage());
                }
            });
            recipient.emplace(r.get(), computer(values));
        });
    };

    void normalize(std::map<Combination, float>& recipient) {
        // get max value
        auto it = recipient.begin();
        float coeff = it->second;
        ++it;
        for (it; it != recipient.end(); ++it) {
            if (it->second > coeff) {
                coeff = it->second;
            }
        }
        // multiply all values x the inverse of the max value
        coeff = 1.f / coeff;
        for (it = recipient.begin(); it != recipient.end(); ++it) {
            it->second *= coeff;
        }
    };

    MessageSum::MessageSum(const distribution::Distribution& factor, const categoric::Group& toMarginalize)
        : Factor(categoric::getComplementary(factor.getGroup(), toMarginalize)) {
        messageBuilder(*this->values, factor, *this->group, [](const std::list<float>& values) {
            float val = 0.f;
            std::for_each(values.begin(), values.end(), [&val](float v) { val += v; });
            return val;
        });
        normalize(*this->values);
    }

    MessageMAP::MessageMAP(const distribution::Distribution& factor, const categoric::Group& toMarginalize)
        : Factor(categoric::getComplementary(factor.getGroup(), toMarginalize)) {
        messageBuilder(*this->values, factor, *this->group, [](const std::list<float>& values) {
            float val = 0.f;
            std::for_each(values.begin(), values.end(), [&val](float v) { if(v > val) val = v; });
            return val;
        });
        normalize(*this->values);
    }
}