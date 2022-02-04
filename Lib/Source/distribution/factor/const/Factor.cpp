/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <Error.h>
#include <XML-Parser/Parser.h>
#include <algorithm>
#include <categoric/Range.h>
#include <distribution/DistributionFinder.h>
#include <distribution/DistributionIterator.h>
#include <distribution/factor/EvaluatorBasic.h>
#include <distribution/factor/const/Factor.h>
#include <fstream>
#include <list>

namespace EFG::distribution::factor::cnst {
Factor::Factor(const std::set<categoric::VariablePtr> &group)
    : DistributionInstantiable(group, std::make_unique<EvaluatorBasic>()) {}

Factor::Factor(const Distribution &o) : Factor(o.getGroup().getVariables()) {
  auto it = o.getIterator();
  iterator::forEach(it, [this](const DistributionIterator &i) {
    this->values->emplace(i.getCombination(), i.getImage());
  });
}

std::set<categoric::VariablePtr>
mergeGroups(const std::set<const Distribution *> &distr) {
  if (distr.size() <= 1) {
    throw Error("you can merge at least 2 distributions");
  }
  auto it = distr.begin();
  std::set<categoric::VariablePtr> merged = (*it)->getGroup().getVariables();
  ++it;
  std::for_each(it, distr.end(), [&merged](const Distribution *d) {
    for (auto itV = d->getGroup().getVariables().begin();
         itV != d->getGroup().getVariables().end(); ++itV) {
      auto itM = merged.find(*itV);
      if (itM == merged.end()) {
        merged.emplace(*itV);
      } else if ((*itM)->size() != (*itV)->size()) {
        throw Error("varaibles with same names should have also the same size");
      }
    }
  });
  return merged;
}
Factor::Factor(const std::set<const Distribution *> &distr)
    : Factor(mergeGroups(distr)) {
  std::list<distribution::DistributionFinder> finders;
  std::for_each(distr.begin(), distr.end(),
                [this, &finders](const Distribution *d) {
                  finders.emplace_back(*d, this->getGroup().getVariables());
                });
  categoric::Range jointDomain(this->getGroup().getVariables());
  float val;
  iterator::forEach(jointDomain, [&](const categoric::Range &jointDomain) {
    val = 1.f;
    for (auto it = finders.begin(); it != finders.end(); ++it) {
      auto result = it->find(jointDomain.get());
      if (nullptr == result.first) {
        val = 0.f;
        break;
      }
      val *= result.second;
    }
    if (0.f != val) {
      this->values->emplace(jointDomain.get(), val);
    }
  });
}

Factor::Factor(const Distribution &toMarginalize,
               const categoric::Combination &comb,
               const std::set<categoric::VariablePtr> &evidences)
    : Factor(getComplementary(toMarginalize.getGroup().getVariables(),
                              evidences)) {
  std::list<std::size_t> indexRemaining, indexEvidence;
  for (std::size_t k = 0; k < toMarginalize.getGroup().getVariables().size();
       ++k) {
    indexRemaining.push_back(k);
  }
  std::for_each(
      evidences.begin(), evidences.end(), [&](const categoric::VariablePtr &v) {
        std::size_t ind =
            std::distance(toMarginalize.getGroup().getVariables().begin(),
                          toMarginalize.getGroup().getVariables().find(v));
        indexEvidence.push_back(ind);
        indexRemaining.remove(ind);
      });

  auto isMatching = [&](const categoric::Combination &combThis) {
    std::size_t k = 0;
    for (auto it = indexEvidence.begin(); it != indexEvidence.end(); ++it) {
      if (combThis.data()[*it] != comb.data()[k]) {
        return false;
      }
      ++k;
    }
    return true;
  };

  auto extractRemainingComb = [&indexRemaining](
                                  const categoric::Combination &combThis) {
    categoric::Combination combRaw(indexRemaining.size());
    auto *dataComb = combRaw.data();
    const auto *dataThis = combThis.data();
    std::size_t k = 0;
    for (auto it = indexRemaining.begin(); it != indexRemaining.end(); ++it) {
      dataComb[k] = dataThis[*it];
      ++k;
    }
    return combRaw;
  };

  auto it = toMarginalize.getIterator();
  iterator::forEach(it, [&](const DistributionIterator &it) {
    if (isMatching(it.getCombination())) {
      this->values->emplace(extractRemainingComb(it.getCombination()),
                            it.getImage());
    }
  });
}

Factor::Factor(const std::set<categoric::VariablePtr> &group, bool corrOrAnti)
    : Factor(group) {
  if (this->group->getVariables().size() <= 1) {
    throw Error(
        "correlating(anti) factor should refer to at least 2 variables");
  }
  // check all variables have same size
  auto itG = this->group->getVariables().begin();
  std::size_t size = (*itG)->size();
  ++itG;
  std::for_each(itG, this->group->getVariables().end(),
                [&size](const categoric::VariablePtr &v) {
                  if (v->size() != size) {
                    throw Error("correlating(anti) should refer to variables "
                                "having all the same size");
                  }
                });

  categoric::Combination comb(this->group->getVariables().size());
  auto *combData = comb.data();
  std::size_t c;
  if (corrOrAnti) {
    // correlation
    for (std::size_t s = 0; s < size; ++s) {
      this->values->emplace(comb, 1.f);
      for (c = 0; c < comb.size(); ++c) {
        combData[c] += 1;
      }
    }
    return;
  }
  // anti correlation
  categoric::Range jointDomain(this->group->getVariables());
  iterator::forEach(jointDomain, [this](categoric::Range &jointDomain) {
    this->values->emplace(jointDomain.get(), 1.f);
  });
  for (std::size_t s = 0; s < size; ++s) {
    this->values->erase(comb);
    for (c = 0; c < comb.size(); ++c) {
      combData[c] += 1;
    }
  }
}

void fillCombination(const std::vector<std::string> &slices,
                     categoric::Combination &comb, float &image) {
  if (slices.size() != (comb.size() + 1)) {
    throw Error("invalid combination");
  }
  auto it = slices.begin();
  for (std::size_t k = 0; k < comb.size(); ++k) {
    comb.data()[k] = static_cast<std::size_t>(std::atoi(it->c_str()));
    ++it;
  }
  image = static_cast<float>(std::atof(it->c_str()));
};
Factor::Factor(const std::set<categoric::VariablePtr> &group,
               const std::string &fileName)
    : Factor(group) {
  std::ifstream reader(fileName);
  if (!reader.is_open()) {
    throw Error("invalid file to import factor");
  }
  categoric::Combination comb(this->group->getVariables().size());
  float image;
  std::string line;
  while (!reader.eof()) {
    std::getline(reader, line);
    auto slices = xmlPrs::slice_fragments(line);
    if (!slices.empty()) {
      // ingore empty lines
      fillCombination(slices, comb, image);
      this->checkCombination(comb, image);
      this->values->emplace(comb, image);
    }
  }
}
} // namespace EFG::distribution::factor::cnst
