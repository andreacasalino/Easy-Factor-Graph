/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <model/RandomField.h>

namespace EFG::model {
    void RandomField::Insert(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert) {
        this->InsertTunableCapable::Insert(toInsert);
        this->insertHandler(toInsert.get());
    }

    void RandomField::Insert(std::shared_ptr<distribution::factor::modif::FactorExponential> toInsert, const categoric::Group& potentialSharingWeight) {
        this->InsertTunableCapable::Insert(toInsert, potentialSharingWeight);
        this->insertHandler(toInsert.get());
    }
}
