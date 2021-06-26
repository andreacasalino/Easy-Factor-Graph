/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <structure/InsertTunableCapable.h>
#include <Error.h>
#include <algorithm>

namespace EFG::strct {
    void InsertTunableCapable::insertTunable(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert) {
        this->InsertCapable::insert(toInsert);
        this->factorsExp.extract(toInsert);
        this->factorsExp.emplace(toInsert, this->numberOfClusters);
        ++this->numberOfClusters;
    }

    void InsertTunableCapable::insertTunableCopy(const distribution::factor::modif::FactorExponential& factor) {
        std::shared_ptr<distribution::factor::modif::FactorExponential> distr = std::make_shared<distribution::factor::modif::FactorExponential>(factor);
        distr->replaceGroup(categoric::Group(this->convertUsingLocals(factor.getGroup().getVariables())));
        this->insertTunable(distr);
    }

    std::map<std::shared_ptr<distribution::factor::modif::FactorExponential>, std::size_t>::const_iterator InsertTunableCapable::findSharingFactor(const std::set<categoric::VariablePtr>& potentialSharingWeight) const {
        for (auto it = this->factorsExp.begin(); it != this->factorsExp.end(); ++it) {
            if (it->first->getGroup().getVariables() == potentialSharingWeight) {
                return it;
            }
        }
        throw Error("inexistent factor for sharing the weight");
    }

    void InsertTunableCapable::insertTunable(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert, const std::set<categoric::VariablePtr>& potentialSharingWeight) {
        auto it = this->findSharingFactor(this->convertUsingLocals(potentialSharingWeight));
        this->InsertCapable::insert(toInsert);
        this->factorsExp.extract(toInsert);
        toInsert->setWeight(it->first->getWeight());
        this->factorsExp.emplace(toInsert, it->second);
    }

    void InsertTunableCapable::insertTunableCopy(const distribution::factor::modif::FactorExponential& factor, const std::set<categoric::VariablePtr>& potentialSharingWeight) {
        std::shared_ptr<distribution::factor::modif::FactorExponential> distr = std::make_shared<distribution::factor::modif::FactorExponential>(factor);
        distr->replaceGroup(categoric::Group(this->convertUsingLocals(factor.getGroup().getVariables())));
        this->insertTunable(distr, potentialSharingWeight);
    }

    void replaceWithCopies(std::vector<std::shared_ptr<distribution::factor::modif::FactorExponential>>& set) {
        std::for_each(set.begin(), set.end(), [](std::shared_ptr<distribution::factor::modif::FactorExponential>& p){
            p = std::make_shared<distribution::factor::modif::FactorExponential>(*p.get());
        });
    };
    void InsertTunableCapable::absorb(const StructureTunableAware& toAbsorb, const bool& useCopyInsertion) {        
        auto clusters = toAbsorb.getFactorsExp();
        if(useCopyInsertion) {
            std::for_each(clusters.begin(), clusters.end(), [](std::vector<std::shared_ptr<distribution::factor::modif::FactorExponential>>& cl){
                replaceWithCopies(cl);
            });
        }
        for(auto it = clusters.begin(); it!=clusters.end(); ++it) {
            auto itCl = it->begin();
            this->insertTunable(*itCl);
            const auto& group = (*itCl)->getGroup().getVariables();
            ++itCl;
            for(itCl; itCl != it->end(); ++itCl) {
                this->insertTunable(*itCl, group);
            }
        }
    }
}
