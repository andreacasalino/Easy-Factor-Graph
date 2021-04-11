/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_NODES_STRUCTURE_AWARE_H
#define EFG_NODES_STRUCTURE_AWARE_H

#include <nodes/Node.h>
#include <nodes/bases/Base.h>
#include <distribution/factor/const/Factor.h>
#include <distribution/factor/const/FactorExponential.h>

namespace EFG::nodes {
    class StructureAware : virtual public Base {
    public:
        inline const std::vector<std::shared_ptr<distribution::factor::cnst::Factor>>& getFactors() const { return this->factors; };
        inline const std::vector<std::shared_ptr<distribution::factor::cnst::FactorExponential>>& getFactorsExp() const { return this->factorsExp; };

    protected:
        std::vector<std::shared_ptr<distribution::factor::cnst::Factor>> factors;
        std::vector<std::shared_ptr<distribution::factor::cnst::FactorExponential>> factorsExp;
    };
}

#endif
