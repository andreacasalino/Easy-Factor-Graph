/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/factor/const/Factor.h>
#include <distribution/factor/EvaluatorBasic.h>
#include <categoric/Range.h>
#include <distribution/DistributionIterator.h>
#include <distribution/DistributionFinder.h>
#include <Error.h>
#include <algorithm>
#include <list>
#include <fstream>
#include <Parser.h>

namespace EFG::distribution::factor::cnst {
    Factor::Factor(const categoric::Group& group)
        : DistributionInstantiable(group, std::make_unique<EvaluatorBasic>()) {
    }

    Factor::Factor(const Distribution& o)
        : Factor(o.getGroup()) {
        auto it = o.getIterator();
        iterator::forEach(it, [this](const DistributionIterator& i) {
            this->values->emplace(i.getCombination(), i.getImage());
        });
    }

    categoric::Group mergeGroups(const std::set<const Distribution*>& distr) {
        if (distr.size() <= 1) {
            throw Error("you can merge at least 2 distributions");
        }
        auto it = distr.begin();
        categoric::Group merged((*it)->getGroup());
        ++it;
        std::for_each(it, distr.end(), [&merged](const Distribution* d) {
            for (auto itV = d->getGroup().getVariables().begin(); itV!= d->getGroup().getVariables().end(); ++itV) {
                try {
                    merged.add(*itV);
                }
                catch (...) {
                }
            }
        });
        return merged;
    }
    Factor::Factor(const std::set<const Distribution*>& distr)
        : Factor(mergeGroups(distr)) {
        std::list<distribution::DistributionFinder> finders;
        std::for_each(distr.begin(), distr.end(), [this, &finders](const Distribution* d) {
            finders.emplace_back(*d, this->getGroup().getVariables());
        });
        categoric::Range jointDomain(this->getGroup());
        iterator::forEach(jointDomain, [this, &finders](const categoric::Range& jointDomain) {
            float val = 1.f;
            Combination comb(jointDomain.get());
            for (auto it = finders.begin(); it != finders.end(); ++it) {
                auto result = it->find(comb);
                if (nullptr == result.first) {
                    val = 0.f;
                    break;
                }
                val *= result.second;
            }
            if (0.f != val) {
                this->values->emplace(comb, val);
            }
        });
    }

    Factor::Factor(const Distribution& toMarginalize, const Combination& comb, const categoric::Group& evidences)
        : Factor(getComplementary(toMarginalize.getGroup(), evidences)) {
        std::list<std::size_t> indexRemaining, indexEvidence;
        for(std::size_t k=0; k<toMarginalize.getGroup().getVariables().size(); ++k) {
            indexRemaining.push_back(k);
        }
        std::for_each(evidences.getVariables().begin(), evidences.getVariables().end(), [&toMarginalize , &indexRemaining, &indexEvidence](const categoric::VariablePtr& v){
            std::size_t ind = std::distance( toMarginalize.getGroup().getVariables().begin(), toMarginalize.getGroup().getVariables().find(v));
            indexEvidence.push_back(ind);
            indexRemaining.remove(ind);
        });

        auto isMatching = [&indexEvidence, &comb](const Combination& combThis){
            std::size_t k=0;
            for (auto it = indexEvidence.begin(); it!=indexEvidence.end(); ++it) {
                if(combThis.data()[*it] != comb.data()[k]) {
                    return false;
                }
                ++k;
            }
            return true;
        };

        auto extractRemainingComb = [&indexRemaining](const Combination& combThis){
            std::vector<std::size_t> combRaw;
            combRaw.reserve(indexRemaining.size());
            for (auto it = indexRemaining.begin(); it!=indexRemaining.end(); ++it) {
                combRaw.push_back(combThis.data()[*it]);
            }
            return Combination(combRaw);
        };

        auto it = toMarginalize.getIterator();
        iterator::forEach(it, [this, &extractRemainingComb, &isMatching](const DistributionIterator& it){
            if (isMatching(it.getCombination())) {
                this->values->emplace(extractRemainingComb(it.getCombination()), it.getImage());
            }
        });
    }

    Factor::Factor(const categoric::Group& group, bool corrOrAnti)
        : Factor(group) {
        if (this->group->getVariables().size() <= 1) {
            throw Error("correlating(anti) factor should refer to at least 2 variables");
        }
        // check all variables have same size
        auto itG = this->group->getVariables().begin();
        std::size_t size = (*itG)->size();
        ++itG;
        std::for_each(itG, this->group->getVariables().end(), [&size](const categoric::VariablePtr& v) {
            if (v->size() != size) {
                throw Error("correlating(anti) should refer to variables having all the same size");
            }
        });

        std::vector<std::size_t> comb(this->group->getVariables().size(), 0);
        if (corrOrAnti) {
            // correlation
            for (std::size_t s = 0; s < size; ++s) {
                this->values->emplace(comb, 1.f);
                std::for_each(comb.begin(), comb.end(), [](std::size_t& v) { v += 1; });
            }
            return;
        }
        // anti correlation
        categoric::Range jointDomain(*this->group);
        iterator::forEach(jointDomain, [this](categoric::Range& jointDomain) {
            this->values->emplace( Combination(jointDomain.get()), 1.f);
        });
        for (std::size_t s = 0; s < size; ++s) {
            this->values->erase(Combination(comb));
            std::for_each(comb.begin(), comb.end(), [](std::size_t& v) { v += 1; });
        }
    }

    void convertCombination(const std::list<std::string>& slices, std::vector<std::size_t>& comb, float& image) {
        if (slices.size() != (comb.size() + 1)) {
            throw Error("invalid combination");
        }
        auto it = slices.begin();
        for (std::size_t k = 0; k<comb.size(); ++k) {
            comb[k] = std::atoi(it->c_str());
            ++it;
        }
        image = static_cast<float>(std::atof(it->c_str()));
    };
    Factor::Factor(const categoric::Group& group, const std::string& fileName)
        : Factor(group) {
        std::ifstream reader(fileName);
        if (!reader.is_open()) {
            throw Error("invalid file to import factor");
        }
        std::vector<std::size_t> combination;
        float image;
        combination.resize(this->group->getVariables().size());
        std::string line;
        while (!reader.eof()) {
            std::getline(reader, line);
            convertCombination(xmlPrs::Parser::sliceFragments(line), combination, image);
            Combination comb(combination);
            this->checkCombination(comb, image);
            this->values->emplace(comb, image);
        }
    }
}
