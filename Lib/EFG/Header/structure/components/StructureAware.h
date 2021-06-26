/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_STRUCTURE_STRUCTURE_AWARE_H
#define EFG_STRUCTURE_STRUCTURE_AWARE_H

#include <structure/Node.h>
#include <Component.h>
#include <distribution/factor/const/Factor.h>
#include <distribution/factor/const/FactorExponential.h>

namespace EFG::strct {
    class StructureAware : virtual public Component {
    public:
        inline const std::set<std::shared_ptr<distribution::factor::cnst::Factor>>& getConstFactors() const { return this->factorsConst; };
        inline const std::set<std::shared_ptr<distribution::factor::cnst::FactorExponential>>& getConstFactorsExp() const { return this->factorsConstExp; };

        inline const std::set<EFG::distribution::DistributionCnstPtr>& getAllFactors() const { return this->factorsAll; };

    protected:
        /**
         * @brief the constant factors stored in the model
         */
        std::set<std::shared_ptr<distribution::factor::cnst::Factor>> factorsConst;
        /**
         * @brief the constant exponential factors stored in the model
         */
        std::set<std::shared_ptr<distribution::factor::cnst::FactorExponential>> factorsConstExp;

        /**
         * @brief a register storing ALL the factors in the model, no matter the kind (exponential, const, non const)
         */
        std::set<EFG::distribution::DistributionCnstPtr> factorsAll;
    };
}

#endif
